<?php

/**
 * @addtogroup database
 * @{
 */

/**
 * @file
 * Query code for MySQL embedded database engine.
 */


class InsertQuery_mysql extends InsertQuery {

  public function execute() {
    if (!$this->preExecute()) {
      return NULL;
    }

    // If we're selecting from a SelectQuery, finish building the query and
    // pass it back, as any remaining options are irrelevant.
    if (empty($this->fromQuery)) {
      $max_placeholder = 0;
      $values = array();
      foreach ($this->insertValues as $insert_values) {
        foreach ($insert_values as $value) {
          $values[':db_insert_placeholder_' . $max_placeholder++] = $value;
        }
      }
    }
    else {
      $values = $this->fromQuery->getArguments();
    }

    $last_insert_id = $this->connection->query((string) $this, $values, $this->queryOptions);

    // Re-initialize the values array so that we can re-use this query.
    $this->insertValues = array();

    return $last_insert_id;
  }

  public function __toString() {
    // Create a sanitized comment string to prepend to the query.
    $comments = $this->connection->makeComment($this->comments);

    // Default fields are always placed first for consistency.
    $insert_fields = array_merge($this->defaultFields, $this->insertFields);

    if (method_exists($this->connection, 'escapeFields')) {
      $insert_fields = $this->connection->escapeFields($insert_fields);
    }

    // If we're selecting from a SelectQuery, finish building the query and
    // pass it back, as any remaining options are irrelevant.
    if (!empty($this->fromQuery)) {
      $insert_fields_string = $insert_fields ? ' (' . implode(', ', $insert_fields) . ') ' : ' ';
      return $comments . 'INSERT INTO {' . $this->table . '}' . $insert_fields_string . $this->fromQuery;
    }

    $query = $comments . 'INSERT INTO {' . $this->table . '} (' . implode(', ', $insert_fields) . ') VALUES ';

    $max_placeholder = 0;
    $values = array();
    if (!is_array($this->insertValues)) {
      if (version_compare(PHP_VERSION, '7.4', '>=')) {
        throw new UnexpectedValueException();
      }
      else {
        drupal_trigger_fatal_error('Unexpected Value');
      }
    }
    if (count($this->insertValues)) {
      foreach ($this->insertValues as $insert_values) {
        $placeholders = array();

        // Default fields aren't really placeholders, but this is the most convenient
        // way to handle them.
        $placeholders = array_pad($placeholders, count($this->defaultFields), 'default');

        $new_placeholder = $max_placeholder + count($insert_values);
        for ($i = $max_placeholder; $i < $new_placeholder; ++$i) {
          $placeholders[] = ':db_insert_placeholder_' . $i;
        }
        $max_placeholder = $new_placeholder;
        $values[] = '(' . implode(', ', $placeholders) . ')';
      }
    }
    else {
      // If there are no values, then this is a default-only query. We still need to handle that.
      $placeholders = array_fill(0, count($this->defaultFields), 'default');
      $values[] = '(' . implode(', ', $placeholders) . ')';
    }

    $query .= implode(', ', $values);

    return $query;
  }
}

class TruncateQuery_mysql extends TruncateQuery { }

class UpdateQuery_mysql extends UpdateQuery {
  public function __toString() {
    if (method_exists($this->connection, 'escapeField')) {
      if (!is_array($this->fields)) {
        if (version_compare(PHP_VERSION, '7.4', '>=')) {
          throw new UnexpectedValueException();
        }
        else {
          drupal_trigger_fatal_error('Unexpected Value');
        }
      }
      $escapedFields = array();
      foreach ($this->fields as $field => $data) {
        $field = $this->connection->escapeField($field);
        $escapedFields[$field] = $data;
      }
      $this->fields = $escapedFields;
    }
    return parent::__toString();
  }
}

/**
 * @} End of "addtogroup database".
 */
