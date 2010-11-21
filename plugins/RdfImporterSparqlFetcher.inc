<?php
// $Id$

/**
 * @file
 * Home of the RdfImporterSparqlFetcher.
 */
 
// The RdfImporterBatch class has been separated so it can
// be used with multiple fetchers.
require_once(dirname(__FILE__) .'/RdfImporterBatch.inc');

/**
 * Class definition for RdfImporterSparqlFetcher.
 */
class RdfImporterSparqlFetcher extends FeedsFetcher {
  
  /**
   * Implementation of FeedsFetcher::fetch().
   */
  public function fetch(FeedsSource $source) {
    $source_config = $source->getConfigFor($this);
    $endpoint = array('remote_store_endpoint' => $source_config['endpoint']);
    $arc = ARC2::getResource();
    $arc->setStore(ARC2::getRemoteStore($endpoint));
    return new RdfImporterBatch($this->getURIs($source_config), $arc);
  }
  
  /**
   * Build a list of URIs from the source config.
   */
  public function getURIs($source_config) {
    $uri_list = array();

    // Gather manually-entered URIs
    if (!empty($source_config['list'])) {
      $items = explode("\n", $source_config['list']);
      if (!empty($items)) {
        foreach ($items as $v) {
          $v = trim($v);
          if (!empty($v) && rdfimporter_validate_uri($v)) {
            $uri_list[$v] = $v;
          }
        }
      }
    }

    // Run the user-provided SPARQL query to extract URIs.
    if (!empty($source_config['query'])) {
      $endpoint = ARC2::getRemoteStore(array('remote_store_endpoint' => $source_config['endpoint']));
      $results = $endpoint->query($source_config['query'], 'rows');
      
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
    $form['query'] = array(
      '#type' => 'textarea',
      '#title' => t('SPARQL query'),
      '#description' => t('Enter a SELECT query that returns URIs. Every returned URI will be imported.'),
      '#default_value' => isset($source_config['query']) ? $source_config['query'] : '',
      '#rows' => 8,
      '#required' => FALSE,
    );
    $form['list'] = array(
      '#type' => 'textarea',
      '#title' => t('URI list'),
      '#description' => t('Enter a list of URIs to be imported from the endpoint. Place each URI on a separate line.'),
      '#default_value' => isset($source_config['list']) ? $source_config['list'] : '',
      '#rows' => 8,
      '#required' => FALSE,
    );
    return $form;
  }
  
  /**
   * Override parent::sourceFormValidate().
   * 
   * @todo Invalid form elements are not getting highlighted
   * @todo Validate query type (SELECT)
   */
  public function sourceFormValidate(&$values) {
    
    // Validate the endpoint.
    if (!empty($values['endpoint']) && !valid_url($values['endpoint'], TRUE)) {
      form_set_error('feeds][endpoint', t('The endpoint must be a valid URL.'));
    }
    
    // Make sure there is at least one field is provided.
    if (empty($values['list']) && empty($values['query'])) {
      form_set_error('feeds][list', t('Either a list of URIs or a SPARQL query is required.'));
      form_set_error('feeds][query', '');
    }
    
    // Validate the list of URIs.
    if (!empty($values['list'])) {
      $items = explode("\n", $values['list']);
      if (!empty($items)) {
        foreach ($items as $v) {
          $v = trim($v);
          if (!empty($v) && !rdfimporter_validate_uri($v)) {
            form_set_error('feeds][list', t('The URI "%v" is not valid.', array('%v' => $v)));
          }
        }
      }
      else {
        form_set_error('feeds][list', t('There is a problem with the list of URIs entered. Make sure they are valid and each is on a separate line.'));
      }
    }
    
    // Validate the SPARQL query.
    if (!empty($values['query'])) {
      if (!rdfimporter_validate_sparql($values['query'], $errors)) {
        form_set_error('feeds][query', t('There is a problem with the SPARQL query. ') . implode('. ', $errors));
      }
    }
  }
}