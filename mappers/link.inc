<?php
// $Id$

/**
 * @file
 * On behalf implementation of Feeds mapping API for link.module.
 */

/**
 * Implementation of hook_rdfimporter_node_processor_targets_alter().
 */
function link_rdfimporter_node_processor_targets_alter(&$targets, $content_type) {
  $new_targets = array();
  link_feeds_node_processor_targets_alter($new_targets, $content_type);
  
  // Change the callback so we can do some preprocessing
  foreach ($new_targets as &$target) {
    $target['callback'] = 'link_rdfimporter_preprocess_target';
  }
  $targets = array_merge($targets, $new_targets);
}

/**
 * Preprocess targets.
 */
function link_rdfimporter_preprocess_target($node, $target, $property) {
  
  // Don't continue if the property is empty.
  if (empty($property)) return;
  
  if (is_string($property) || is_array($property)) {
    return link_feeds_set_target($node, $target, $property);
  }
  
  list($field_name, $sub_field) = split(':', $target);
  
  $values = array();
  $i = 0;
  foreach ($property->getValues() as $item) {
    if ($item->getType() == 'uri') {
      $values[$i] = ($sub_field == 'url') ? $item->getValue() : $item->getLabel();
    }
    else {
      $values[$i] = $item->getValue();
    }
    $i++;
  }
  link_feeds_set_target($node, $target, $values);
}