<?php

/**
 * @file
 * Mapper that exposes a node's taxonomy vocabularies as mapping targets.
 */

/**
 * Implementation of hook_rdfimporter_node_processor_targets_alter().
 */
function taxonomy_rdfimporter_node_processor_targets_alter(&$targets, $content_type) {
  $new_targets = array();
  taxonomy_feeds_node_processor_targets_alter($new_targets, $content_type);
  
  // Change the callback so we can do some preprocessing
  foreach($new_targets as &$target) {
    $target['callback'] = 'taxonomy_rdfimporter_preprocess_target';
  }
  $targets = array_merge($targets, $new_targets);
}

/**
 * Preprocess targets.
 *
 * If the vocabulary is set up for tags we need to wrap terms 
 * inside double quotes so that they get parsed correctly.
 * Feeds will implode tags using commas, so wrapping with double
 * quotes ensures terms with commas get handled correctly.
 */
function taxonomy_rdfimporter_preprocess_target(&$node, $key, $property) {
  
  // Don't continue if the property is empty.
  if (empty($property)) return;
  
  if (is_string($property)) {
    $property = check_plain($property);
    return ($property) ? taxonomy_feeds_set_target($node, $key, '"'.$property.'"') : NULL;
  }
  
  // Load target vocabulary to check if it has the "tags" flag.
  $vocab_id = (int) str_replace('taxonomy:', '', $key);
  $vocab = taxonomy_vocabulary_load($vocab_id);

  $terms = array();
  foreach($property->getValues() as $item) {
    $value = ($item->getType() == 'uri') ? $item->getLabel() : $item->getValue();
    $value = check_plain($value);

    if ($value && $vocab->tags) {
      $value = '"'.$value.'"';
    }
    $terms[] = $value;
  }
  return taxonomy_feeds_set_target($node, $key, $terms);
}