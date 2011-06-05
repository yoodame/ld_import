<?php
// $Id$

/**
 * @file
 * Home of the RdfImporterSparqlFetcher.
 */

// The RdfImporterFetcherResult class has been separated so it can
// be used with multiple fetchers.
require_once(dirname(__FILE__) .'/RdfImporterFetcherResult.inc');

/**
 * Class definition for RdfImporterSparqlFetcher.
 */
class RdfImporterSparqlFetcher extends FeedsFetcher {

  /**
   * Implementation of FeedsFetcher::fetch().
   */
  public function fetch(FeedsSource $source) {
    $source_config = $source->getConfigFor($this);
    $endpoint = array(
      'remote_store_endpoint' => $source_config['endpoint'],
    );
    $arc = ARC2::getResource();
    $arc->setStore(ARC2::getRemoteStore($endpoint));

    if ($this->config['enable_data_queries'] && !empty($source_config['data_query'])) {
      $this->getSourceData($source_config['data_query'], $arc);
    }

    return new RdfImporterFetcherResult($this->getURIs($source_config), $arc);
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

  public function getSourceData($query, $arc) {
    $index = $arc->store->query($query, 'raw');
    if (count($index) > 1) {
      $arc->index = ARC2::getMergedIndex($index);
      $arc->fetched = array_keys($index);
    }
    else {
      $errors = !empty($index['errors']) ? theme_item_list($index['errors']) : NULL;
      throw new Exception(t('Could not retrieve any data using the provided data query. !errors', array('!errors' => $errors)));
    }
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
    $form['divider'] = array(
      '#prefix' => '<p><em>',
      '#value' => t('or...'),
      '#suffix' => '</em><p>',
    );
    $form['list'] = array(
      '#type' => 'textarea',
      '#title' => t('URI list'),
      '#description' => t('Enter a list of URIs to be imported from the endpoint. Place each URI on a separate line.'),
      '#default_value' => isset($source_config['list']) ? $source_config['list'] : '',
      '#rows' => 6,
      '#required' => FALSE,
    );
    $form['data_query'] = array(
      '#type' => ($this->config['enable_data_queries']) ? 'textarea' : 'hidden',
      '#title' => t('Data query'),
      '#description' => t('Enter a CONSTRUCT query that returns a graph with all data required for this import.'),
      '#default_value' => isset($source_config['data_query']) ? $source_config['data_query'] : '',
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
      form_set_error('feeds][RdfImporterSparqlFetcher][endpoint', t('The endpoint must be a valid URL.'));
    }

    // Make sure there is at least one field is provided.
    if (empty($values['list']) && empty($values['list_query'])) {
      form_set_error('feeds][RdfImporterSparqlFetcher][list', t('Either a list of URIs or a SPARQL query is required.'));
      form_set_error('feeds][RdfImporterSparqlFetcher][list_query', '');
    }

    // Validate the list of URIs.
    if (!empty($values['list'])) {
      $items = explode("\n", $values['list']);
      if (!empty($items)) {
        foreach ($items as $v) {
          $v = trim($v);
          if (!empty($v) && !rdfimporter_validate_uri($v)) {
            form_set_error('feeds][RdfImporterSparqlFetcher][list', t('The URI "%v" is not valid.', array('%v' => $v)));
          }
        }
      }
      else {
        form_set_error('feeds][RdfImporterSparqlFetcher][list', t('There is a problem with the list of URIs entered. Make sure they are valid and each is on a separate line.'));
      }
    }

    // Validate the SPARQL query.
    if (isset($values['list_query']) && !empty($values['list_query'])) {
      $errors = array();
      if (! rdfimporter_validate_sparql($values['list_query'], $errors) ) {
        form_set_error('feeds][RdfImporterSparqlFetcher][list_query', t('There is a problem with the SPARQL query.') . theme_item_list($errors));
      }
    }
    // Validate the SPARQL query.
    if (isset($values['data_query']) && !empty($values['data_query'])) {
      $errors = array();
      if (!rdfimporter_validate_sparql($values['data_query'], $errors)) {
        form_set_error('feeds][RdfImporterSparqlFetcher][data_query', t('There is a problem with the data query.') . theme_item_list($errors));
      }
    }
  }

  /**
   * Override parent::configDefaults().
   */
  public function configDefaults() {
    return array(
      'enable_data_queries' => 0,
    );
  }

  /**
   * Override parent::configForm().
   */
  public function configForm($form_state) {
    $form['enable_data_queries'] = array(
      '#type' => 'radios',
      '#title' => t('Enable data queries'),
      '#description' => t('<p>Allow a query to be provided that retrieves a graph of all data necessary for import. This can make large imports MUCH more efficient. Normally, separate requests will be made to fetch <em>all properties</em> for each individual being mapped (both subjects and objects). This means a lot of HTTP requests and a lot of unnecessary data. By using a data query, only two requests are made: one for the list of URIs to import and one for the data needed for configured mappings.</p><p>Note: When a data query is provided, previews will also use that data.</p><p>Also: Be sure to set a large batch size under Processor -> Settings.</p>'),
      '#options' => array(
        1 => 'Yes',
        0 => 'No',
      ),
      '#default_value' => $this->config['enable_data_queries'],
    );
    return $form;
  }
}