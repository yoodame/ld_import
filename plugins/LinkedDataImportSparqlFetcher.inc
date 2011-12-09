<?php

/**
 * @file
 * Home of the LinkedDataImportSparqlFetcher.
 */

require_once(dirname(__FILE__) . '/LinkedDataImportFetcherResult.inc');

/**
 * Class definition for LinkedDataImportSparqlFetcher.
 */
class LinkedDataImportSparqlFetcher extends LinkedDataImportFetcher {

  /**
   * Build a list of URIs from the source config.
   */
  public function getURIs(FeedsSource $source) {
    $source_config = $source->getConfigFor($this);
    
    $uri_list = array();

    // Gather manually-entered URIs
    if (!empty($source_config['list'])) {
      $items = explode("\n", $source_config['list']);
      if (!empty($items)) {
        foreach ($items as $v) {
          $v = trim($v);
          if (!empty($v) && ld_import_validate_uri($v)) {
            $uri_list[$v] = $v;
          }
        }
      }
    }

    // Run the user-provided SPARQL query to extract URIs.
    if (!empty($source_config['list_query'])) {
      $endpoint = ARC2::getRemoteStore(array('remote_store_endpoint' => $source_config['endpoint']));
      $results = $endpoint->query($source_config['list_query'], 'rows');

      // Loop through the results and extract every value that's a URI.
      foreach ($results as $row) {
        foreach ($row as $key => $value) {
          if (isset($row[$key . ' type']) && $row[$key . ' type'] == 'uri') {
            $uri_list[$value] = $value;
          }
        }
      }
    }
    return array_values($uri_list);
  }
  
  public function getArc(FeedsSource $source)  {
    $source_config = $source->getConfigFor($this);
    $endpoint = array(
      'remote_store_endpoint' => $source_config['endpoint'],
    );
    $arc = ARC2::getResource();
    $arc->setStore(ARC2::getRemoteStore($endpoint));
    return $arc;
  }

  /**
   * Expose source form.
   *
   * @todo Provide a way to test the SPARQL query
   * @todo Inline Javascript validation
   */
  public function sourceForm($source_config) {
    $form = array();
    $form['endpoint'] = array(
      '#type' => 'textfield',
      '#title' => t('Endpoint'),
      '#description' => t('Enter a SPARQL endpoint URL.'),
      '#default_value' => isset($source_config['endpoint']) ? $source_config['endpoint'] : '',
      '#maxlength' => NULL,
      '#required' => TRUE,
      '#size' => 100,
    );
    $form['list_query'] = array(
      '#type' => 'textarea',
      '#title' => t('SPARQL query'),
      '#description' => t('Enter a SELECT query that returns URIs. Every returned URI will be imported.'),
      '#default_value' => isset($source_config['list_query']) ? $source_config['list_query'] : '',
      '#rows' => 6,
      '#required' => FALSE,
    );
    $form['list'] = array(
      '#type' => 'textarea',
      '#title' => t('URI list'),
      '#description' => t('Enter a list of URIs to be imported from the endpoint. Place each URI on a separate line.'),
      '#default_value' => isset($source_config['list']) ? $source_config['list'] : '',
      '#rows' => 6,
      '#required' => FALSE,
    );
    return $form;
  }

  /**
   * Override parent::sourceFormValidate().
   *
   * @todo Validate query type (SELECT)
   */
  public function sourceFormValidate(&$values) {

    // Validate the endpoint.
    if (!empty($values['endpoint']) && !valid_url($values['endpoint'], TRUE)) {
      form_set_error('feeds][' . get_class($this) . '][endpoint', t('The endpoint must be a valid URL.'));
    }

    // Make sure there is at least one field is provided.
    if (empty($values['list']) && empty($values['list_query'])) {
      form_set_error('feeds][' . get_class($this) . '][list_query', t('Either a list of URIs or a SPARQL query is required.'));
      form_set_error('feeds][' . get_class($this) . '][list', '');
    }

    // Validate the list of URIs.
    if (!empty($values['list'])) {
      $items = explode("\n", $values['list']);
      if (!empty($items)) {
        foreach ($items as $v) {
          $v = trim($v);
          if (!empty($v) && !ld_import_validate_uri($v)) {
            form_set_error('feeds][' . get_class($this) . '][list', t('The URI "%v" is not valid.', array('%v' => $v)));
          }
        }
      }
      else {
        form_set_error('feeds][' . get_class($this) . '][list', t('There is a problem with the list of URIs entered. Make sure they are valid and each is on a separate line.'));
      }
    }

    // Validate the SPARQL query.
    if (isset($values['list_query']) && !empty($values['list_query'])) {
      $errors = array();
      if (! ld_import_validate_sparql($values['list_query'], $errors) ) {
        form_set_error('feeds][' . get_class($this) . '][list_query', t('There is a problem with the SPARQL query.') . theme_item_list($errors));
      }
    }
  }
}
