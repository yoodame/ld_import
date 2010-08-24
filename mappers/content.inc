<?php
// $Id$

/**
 * @file
 * On behalf implementation of Feeds mapping API for content.module (CCK).
 */

/**
 * Implementation of hook_rdfimporter_node_processor_targets_alter().
 */
function content_rdfimporter_node_processor_targets_alter(&$targets, $content_type) {
  $new_targets = array();
  content_feeds_node_processor_targets_alter($new_targets, $content_type);
  
  // Change the callback so we can do some preprocessing
  foreach ($new_targets as &$target) {
    $target['callback'] = 'content_rdfimporter_preprocess_target';
  }
  $targets = array_merge($targets, $new_targets);
}

/**
 * Preprocess targets.
 */
function content_rdfimporter_preprocess_target(&$node, $target, $property) {
  
  // Don't continue if the property is empty.
  if (empty($property)) return;
  
  if (is_string($property)) {
    return content_feeds_set_target($node, $target, $property);
  }
  
  $values = array();
  foreach ($property->getValues() as $item) {
    $value = ($item->getType() == 'uri') ? $item->getLabel() : $item->getValue();
    if (trim($value)) {
      $values[] = $value;
    }
  }
  content_feeds_set_target($node, $target, $values);
}