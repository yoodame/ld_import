<?php
// $Id$

/**
 * @file
 * Class definition of FeedsNodeProcessor.
 */

/**
 * Creates nodes from RdfImporterIndividual objects.
 */
class RdfImporterNodeProcessor extends FeedsNodeProcessor {

  /**
   * Implementation of FeedsProcessor::process().
   */
  public function process(FeedsImportBatch $batch, FeedsSource $source) {
    
    // If this is a 'preview' import don't do any processing
    if ($source->preview) return;
    else parent::process($batch, $source);
    
    // Clear out the ARC index to avoid memory limit issues
    // $batch->arc->index = array();
  }

  /**
   * Override parent::map() to load all available add-on mappers.
   */
  protected function map($individual, $target_node) {
    // Setting the title here, but it can be overridden by mappings
    $target_node->title = $individual->getLabel();
    // Setting the URI to act as Feeds identifier
    $target_node->feeds_node_item->url = $individual->getUri();
    parent::loadMappers();
    self::loadMappers();
    return parent::map($individual, $target_node);
  }
  
  /**
   * Override setTargetElement to only map title and body fields.
   */
  public function setTargetElement($target_node, $target_element, $value) {
    if ($target_element == 'body') {
      $target_node->teaser = node_teaser($value);
      $target_node->body = $value;
    }
    elseif ($target_element == 'title') {
      $target_node->title = $value;
    }
  }

  /**
   * Return available mapping targets.
   */
  public function getMappingTargets() {
    $targets = array(
      'title' => array(
        'name' => t('Title'),
        'description' => t('The title of the node.'),
       ),
     );
    // Include body field only if available.
    $type = node_get_types('type',  $this->config['content_type']);
    if ($type->has_body) {
      // Using 'teaser' instead of 'body' forces entire content above the break.
      $targets['body'] = array(
        'name' => t('Body'),
        'description' => t('The body of the node. The teaser will be the same as the entire body.'),
      );
    }

    // Load mappers bundled with Feeds
    parent::loadMappers();
    // Load RDFimporter mappers
    self::loadMappers();
    
    // Let other modules expose mapping targets.
    drupal_alter('rdfimporter_node_processor_targets', $targets, $this->config['content_type']);

    return $targets;
  }
  
  /**
   * Get nid of an existing feed item node if available.
   */
  protected function existingItemId($individual, FeedsSource $source) {
    $nid = db_result(db_query("SELECT nid FROM {feeds_node_item} WHERE feed_nid = %d AND id = '%s' AND url = '%s'", $source->feed_nid, $source->id, $individual->getUri()));
    return ($nid) ? $nid : 0;
  }
  
  /**
   * Loads available mappers from mappers/ directory.
   */
  protected static function loadMappers() {
    static $loaded = FALSE;
    if (!$loaded) {
      $path = drupal_get_path('module', 'rdfimporter') .'/mappers';
      $files = drupal_system_listing('.*\.inc$', $path, 'name', 0);
      foreach ($files as $file) {
        if (strstr($file->filename, '/mappers/')) {
          require_once("./$file->filename");
        }
      }
      // Rebuild cache.
      module_implements('', FALSE, TRUE);
    }
    $loaded = TRUE;
  }
}