<?php
// $Id$

/**
 * @file
 * Class definition of FeedsNodeProcessor.
 */

// NOTE: The batch size must be greater than 1 otherwise
// progress gets calculated incorrectly.
define('RDFIMPORTER_NODE_BATCH_SIZE', 10);

/**
 * Class definition for RdfImporterNodeProcessor.
 */
class RdfImporterNodeProcessor extends FeedsNodeProcessor {

  /**
   * Implementation of FeedsProcessor::process().
   */
  public function process(FeedsImportBatch $batch, FeedsSource $source) {
    // We override the FEEDS_NODE_BATCH_SIZE by temporarily setting
    // the 'feeds_node_batch_size' variable. This is a little dirty since
    // it's unclear who else might be setting feeds_node_batch_size and where.
    variable_set('feeds_node_batch_size', $this->config['batch_size']);
    
    parent::process($batch, $source);
    
    variable_set('feeds_node_batch_size', NULL);
    
    // If caching is disabled, clear out the ARC index.
    if (!$this->config['arc_caching'] && is_object($batch->arc)) {
      $batch->arc->index = array();
      $batch->arc->fetched = array();
    }
  
    // Delete the temp file if processing is complete.
    if (!$batch->getItemCount()) {
      file_delete($batch->temp_file);
    }
  }
  
  /**
   * Override parent::map() to add some defaults.
   */
  protected function map($batch, $target_node) {
    $individual = $batch->currentItem();
    // Setting the rdfs:label as default title here, but it can be overridden with mappings.
    $target_node->title = $individual->getLabel();
    // Setting the URI to act as Feeds identifier.
    $target_node->feeds_node_item->url = $individual->getUri();
    parent::loadMappers();
    return parent::map($batch, $target_node);
  }
  
  /**
   * Override setTargetElement to only map title and body fields.
   */
  public function setTargetElement($target_node, $target_element, $value) {
    // If we have a multi-value array, use the first value.
    if (is_array($value)) {
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
   * Override parent::configDefaults().
   */
  public function configDefaults() {
    $defaults = parent::configDefaults();
    $defaults['arc_caching'] = 1;
    $defaults['batch_size'] = RDFIMPORTER_NODE_BATCH_SIZE;
    return $defaults;
  }
  
  /**
   * Override parent::configForm().
   */
  public function configForm($form_state) {
    $form = parent::configForm($form_state);
    $form['arc_caching'] = array(
      '#type' => 'radios',
      '#title' => t('Cache data during import'),
      '#description' => t('Normally RDF content will be cached as it is retrieved the server. For large imports this can cause PHP to hit memory limits. <strong>Turning off caching can make imports slower, but overall more reliable.</strong>'),
      '#options' => array(
        1 => 'Enabled',
        0 => 'Disabled',
      ),
      '#default_value' => $this->config['arc_caching'],
    );
    $form['batch_size'] = array(
      '#type' => 'select',
      '#title' => t('Batch size'),
      '#description' => t('Number of URIs to fetch and process into nodes at a time (using Batch API). Setting this higher can make large imports faster, but can lead to PHP timeouts depending on your max_execution_time setting.'),
      '#options' => drupal_map_assoc(array(2,3,4,5,6,7,8,9,10,15,20,30,40,50,100)),
      '#default_value' => $this->config['batch_size'],
    );
    return $form;
  }
}