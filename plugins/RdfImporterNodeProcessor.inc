<?php
// $Id$

/**
 * @file
 * Class definition of FeedsNodeProcessor.
 */

// NOTE: The batch size must be greater than 1 otherwise
// progress gets calculated incorrectly.
define('RDFIMPORTER_NODE_BATCH_SIZE', 2);

/**
 * Class definition for RdfImporterNodeProcessor.
 */
class RdfImporterNodeProcessor extends FeedsNodeProcessor {

  /**
   * Implementation of FeedsProcessor::process().
   */
  public function process(FeedsImportBatch $batch, FeedsSource $source) {

    // We override the FEEDS_NODE_BATCH_SIZE to use by RDFIMPORTER_NODE_BATCH_SIZE
    // temporarily setting the 'feeds_node_batch_size' variable
    // This is a little dirty, but it works for now.
    $original_batch_size = variable_get('feeds_node_batch_size', NULL);
    variable_set('feeds_node_batch_size', RDFIMPORTER_NODE_BATCH_SIZE);
    
    parent::process($batch, $source);
    
    // If caching is disabled, clear out the ARC index.
    if (!$this->config['arc_caching'] && is_object($batch->arc)) {
      $batch->arc->index = array();
    }
    
    variable_set('feeds_node_batch_size', $original_batch_size);
  }
  
  /**
   * Implementation of FeedsProcessor::clear().
   */
  public function clear(FeedsBatch $batch, FeedsSource $source) {
    variable_set('feeds_node_batch_size', FEEDS_NODE_BATCH_SIZE);
    parent::clear($batch, $source);
  }
  
  /**
   * Override parent::map() to load all available add-on mappers.
   */
  protected function map($batch, $target_node) {
    $individual = $batch->currentItem();
    // Setting a default title here, but it can be overridden by mappings
    $target_node->title = $individual->getLabel();
    // Setting the URI to act as Feeds identifier
    $target_node->feeds_node_item->url = $individual->getUri();
    parent::loadMappers();
    return parent::map($batch, $target_node);
  }
  
  /**
   * Override setTargetElement to only map title and body fields.
   */
  public function setTargetElement($target_node, $target_element, $value) {
    if (is_object($value)) {
      $prop = reset($value->getValues());
      $value = (!empty($prop)) ? $prop->getContent() : '';
    }
    elseif (is_array($value)) {
      $value = (!empty($value)) ? reset($value) : '';
    }
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
    // Load mappers bundled with Feeds.
    parent::loadMappers();
    
    // Let other modules expose mapping targets.
    drupal_alter('feeds_node_processor_targets', $targets, $this->config['content_type']);
    drupal_alter('rdfimporter_node_processor_targets', $targets, $this->config['content_type']);
    
    return $targets;
  }
  
  /**
   * Get nid of an existing feed item node if available.
   */
  protected function existingItemId(FeedsImportBatch $batch, FeedsSource $source) {
    $individual = $batch->currentItem();
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
  
  /**
   * Override parent::configDefaults().
   */
  public function configDefaults() {
    $defaults = parent::configDefaults();
    $defaults['arc_caching'] = 0;
    return $defaults;
  }
  
  /**
   * Override parent::configForm().
   */
  public function configForm(&$form_state) {
    $form = parent::configForm($form_state);
    $form['arc_caching'] = array(
      '#type' => 'radios',
      '#title' => t('Cache data during import'),
      '#description' => t('Normally RDF content will be cached as it is retrieved the server. For big imports this can cause PHP to hit memory limits or cause MySQL to hit packet size limits (the cache gets temporarily saved to the database during import). <strong>Turning off caching can make imports slower, but overall more reliable.</strong>'),
      '#options' => array(
        1 => 'Enabled',
        0 => 'Disabled',
      ),
      '#default_value' => $this->config['arc_caching'],
    );
    return $form;
  }
}