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
    if ($target_item) {
      $individual = $result->currentItem();

      // Setting the rdfs:label as default title here, but it can be overridden with mappings.
      $target_item->title = $individual->getLabel();
      $target_item->feeds_item->url = $individual->getUri();
    }
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
   * Override mapping targets to add our own hook.
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
   * Override parent::existingEntityId() since don't use unique targets, just the URI with url field.
   */
  protected function existingEntityId(FeedsSource $source, FeedsParserResult $result) {
    $query = db_select('feeds_item')
      ->fields('feeds_item', array('entity_id'))
      ->condition('feed_nid', $source->feed_nid)
      ->condition('entity_type', $this->entityType())
      ->condition('id', $source->id);
      
    $uri = $result->current_item->getUri();
    $entity_id = $query->condition('url', $uri)->execute()->fetchField();

    return ($entity_id) ? $entity_id : 0;
  }

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
