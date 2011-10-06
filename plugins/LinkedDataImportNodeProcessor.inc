<?php
// $Id$

/**
 * @file
 * Class definition of FeedsNodeProcessor.
 */

// NOTE: The batch size must be greater than 1 otherwise
// progress gets calculated incorrectly.
define('LD_IMPORT_NODE_BATCH_SIZE', 10);

/**
 * Class definition for LinkedDataImportNodeProcessor.
 */
class LinkedDataImportNodeProcessor extends FeedsNodeProcessor {

  /**
   * Override parent::process().
   */
  public function process(FeedsSource $source, FeedsParserResult $parser_result) {
    parent::process($source, $parser_result);

    // If caching is disabled, clear out the ARC index.
    if (!$this->config['arc_caching'] && is_object($batch->arc)) {
      $batch->arc->index = array();
      $batch->arc->fetched = array();
    }

    // Delete the temp file if processing is complete.
    if (! count($parser_result->items)) {
      file_unmanaged_delete($parser_result->temp_file);
    }
  }

  /**
   * Override parent::map() to add some defaults.
   */
  protected function map(FeedsSource $source, FeedsParserResult $result, $target_item = NULL) {
    $individual = $result->currentItem();

    // Setting the rdfs:label as default title here, but it can be overridden with mappings.
    $target_item->title = $individual->getLabel();

    parent::loadMappers();
    return parent::map($source, $result, $target_item);
  }

  /**
   * Override setTargetElement to only take first value from multi-valued fields.
   */
  public function setTargetElement(FeedsSource $source, $target_item, $target_element, $value) {
    // If we have a multi-value array, use the first value.
    if (is_array($value)) {
      $value = (!empty($value)) ? array_shift($value) : '';
    }
    parent::setTargetElement($source, $target_item, $target_element, $value);
  }

  /**
   * Return available mapping targets.
   */
  public function getMappingTargets() {
    $type = node_type_get_type($this->config['content_type']);
    $targets = parent::getMappingTargets();

    // Load mappers bundled with Feeds.
    self::loadMappers();

    // Let other modules expose mapping targets.
    feeds_alter('ld_import_node_processor_targets', $targets, 'node', $this->config['content_type']);

    return $targets;
  }

  /**
   * Get nid of an existing feed item node if available.
   */
  /*
  protected function existingItemId(FeedsImportBatch $batch, FeedsSource $source) {
    $individual = $batch->currentItem();
    $nid = db_result(db_query("SELECT nid FROM {feeds_node_item} WHERE feed_nid = %d AND id = '%s' AND url = '%s'", $source->feed_nid, $source->id, $individual->getUri()));
    return ($nid) ? $nid : 0;
  }
  */

  /**
   * Override parent::configDefaults().
   */
  public function configDefaults() {
    $defaults = parent::configDefaults();
    $defaults['arc_caching'] = 1;
    $defaults['batch_size'] = LD_IMPORT_NODE_BATCH_SIZE;
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