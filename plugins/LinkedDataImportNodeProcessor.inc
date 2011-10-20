<?php

/**
 * @file
 * Home of the LinkedDataImportNodeProcessor.
 */

/**
 * Class definition for LinkedDataImportNodeProcessor.
 */
class LinkedDataImportNodeProcessor extends FeedsNodeProcessor {
  
  /**
   * Override parent::map() to set some defaults before mapping.
   */
  protected function map(FeedsSource $source, FeedsParserResult $result, $target_item = NULL) {
    if ($target_item) {
      $individual = $result->currentItem();
      // Setting the rdfs:label as default title here, but it can 
      // be overridden with mappings.
      $target_item->title = $individual->getLabel();
      $target_item->feeds_item->url = $individual->getUri();
    }
    parent::loadMappers();
    return parent::map($source, $result, $target_item);
  }
  
  /**
   * Override parent::existingEntityId() since we don't use unique targets,
   * just the URI with url field.
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
   * Override parent::getLimit() to use config value instead of system variable.
   */
  public function getLimit() {
    return $this->config['batch_size'];
  }
  
  /**
   * Override parent::configDefaults() to add default batch size.
   */
  public function configDefaults() {
    $defaults = parent::configDefaults();
    $defaults['batch_size'] = 1;
    return $defaults;
  }

  /**
   * Override parent::configForm() to add batch size option.
   */
  public function configForm($form_state) {
    $form = parent::configForm($form_state);
    $form['batch_size'] = array(
      '#type' => 'select',
      '#title' => t('Batch size'),
      '#description' => t('Number of URIs to fetch and process into nodes at a time (using Batch API). Setting this higher can make large imports faster, but can lead to PHP timeouts depending on your max_execution_time setting.'),
      '#options' => drupal_map_assoc(array(1,2,3,4,5,6,7,8,9,10,15,20,30,40,50,100)),
      '#default_value' => $this->config['batch_size'],
    );
    return $form;
  }
  
  /**
   * Override parent::hash() to work on LinkedDataImportResource items.
   */
  protected function hash($item) {
    static $serialized_mappings;
    if (!$serialized_mappings) {
      $serialized_mappings = serialize($this->config['mappings']);
    }
    // Trigger fetching of data if not already fetched.
    $item->hasData();
    // Extract the data fetched by ARC for only this item.
    $serialized_data = serialize($item->getArc()->index[$item->getUri()]);
    return hash('md5', $serialized_data . $serialized_mappings);
  }
  
}
