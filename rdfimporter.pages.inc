<?php

/**
 * Render a feed preview table with a form to refresh sample content
 */
function rdfimporter_preview_tab_form(&$form_state, $node) {
  drupal_add_css(drupal_get_path('module', 'rdfimporter') . '/rdfimporter.css');
  
  // Get a list of the plugins currently offered by this module
  $plugins = array_keys(rdfimporter_feeds_plugins());
  
  // Gather all the importers attached to this node type
  // and all the feed sources this node represents
  $sources = array();
  foreach (feeds_importer_load_all() as $importer) {
    if ($importer->config['content_type'] == $node->type && in_array(get_class($importer->fetcher), $plugins)) {
      $sources[] = feeds_source($importer->id, $node->nid);
    }
  }

  // Build a form for each Feeds source
  $forms = array();
  foreach ($sources as $source) {
    
    $source_config = $source->getConfig();
    
    $form = array(
      '#type' => 'fieldset',
      '#title' => 'Importer: '. l($source->importer->config['name'], 'admin/build/feeds/edit/'.$source->importer->id.'/mapping'),
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
    );
    
    $form['refresh'] = array(
      '#type' => 'submit',
      '#value' => t('Refresh preview'),
      '#submit' => array('rdfimporter_refresh_preview_submit'),
      '#executes_submit_callback' => TRUE,
      '#weight' => 40,
    );

    // Store values to be used by refresh submit handler
    $form['feed_nid'] = array(
      '#type' => 'value',
      '#value' => $node->nid,
    );
    $form['importer'] = array(
      '#type' => 'value',
      '#value' => $source->importer->id,
    );
    
    $form['title'] = array(
      '#type' => 'item',
      '#title' => t('Feed preview'),
    );
    
    $parser = get_class($source->importer->parser);
    
    // Load sample data if available
    if (count($source_config[$parser]['samples'])) {
      
      $form['title']['#description'] = t('Last updated '. format_interval(time() - $source_config[$parser]['last_updated'], 2) . ' ago');
      
      $form['content'] = array(
        '#value' => theme('rdfimporter_samples', $source_config[$parser]['samples']),
        '#weight' => 30,
      );
      $form['list'] = array(
        '#type' => 'fieldset',
        '#title' => t('Feed URIs'),
        '#description' => t('URIs used to fetch this sample'),
        '#value' => theme_item_list($source_config[$parser]['uri_list']),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#weight' => 35,
      );
    }
    else {
      $form['content'] = array(
        '#value' => t('There is no sample content saved for this feed.'),
        '#prefix' => '<p>',
        '#suffix' => '</p>',
        '#weight' => 30,
      );
    }
    
    $form['message'] = array(
      '#type' => 'item',
      '#description' => t('Note: Refreshing fetches ALL feed content. This can take a while.'),
      '#weight' => 50,
    );
  }
  $forms[$source->importer->id] = $form;
  
  return $forms;
}

/**
 * Submit handler for feed preview 'Refresh' button
 *
 * Sets up a batch job to do a 'preview' import
 */
function rdfimporter_refresh_preview_submit($form, $form_state) {
  $batch = array(
    'operations' => array(
      array('rdfimporter_refresh_preview_process', array($form_state['values']['importer'], $form_state['values']['feed_nid'])),
      ),
    'title' => t('Refreshing feed preview'),
    'init_message' => t('Fetching data...'),
    'progress_message' => t('Fetching data...'),
    'error_message' => t('Could not finish fetching data.'),
    'file' => drupal_get_path('module', 'rdfimporter') . '/rdfimporter.pages.inc',
  );
  batch_set($batch);
}

/**
 * Batch operation for feed preview,
 * runs the feed import until finished
 *
 * This is almost the same as feeds_batch() but we're
 * setting a 'preview' flag so nodes aren't saved
 */
function rdfimporter_refresh_preview_process($importer_id, $feed_nid, &$context) {
  $context['finished'] = 1;
  try {
    $source = feeds_source($importer_id, $feed_nid);
    $source->preview = TRUE;
    $context['finished'] = $source->import();
  }
  catch (Exception $e) {
    drupal_set_message($e->getMessage(), 'error');
  }
}

/**
 * Theme sample data for a feed as a table
 */
function theme_rdfimporter_samples($samples) {
  if (count($samples)) {
    $rows = array();
    
    foreach ($samples as $uri => $property) {
      $row = $items = array();
      $row[0] = array();
      // If the sample array has been merged with others the label may an array
      $label = is_array($property['label']) ? $property['label'][0] : $property['label'];
      $row[0]['data'] .= '<strong class="property-label">'. $label .'</strong>';
      $row[0]['data'] .= '<em class="property-uri">'. $uri .'</em>';
      $row[0]['class'] = 'property';
      
      // Clean up the array - if it has been merged it may have duplicates
      // $property['values'] = array_unique($property['values']);
      $property['values'] = array_slice($property['values'], 0, RDFIMPORTER_PARSER_MAX_SAMPLES);
      
      // Build a list of property values
      foreach ($property['values'] as $i => $item) {
        $items[$i] = theme('rdfimporter_sample', $item);
      }
      
      $row[1] = theme('item_list', $items);
      $rows[] = $row;
    }
    
    // Sort by property name
    uasort($rows, 'rdfimporter_sort_table_rows');
    $header = array(
      '0' => array('data' => t('RDF Property'), 'class' => 'first'),
      '1' => array('data' => t('Sample'), 'class' => 'second'),
    );
    
    return theme('table', $header, $rows, array('class' => 'rdfimporter-samples'));
  }
  else {
    return '';
  }
}

/**
 * Theme a single sample
 */
function theme_rdfimporter_sample($item) {
  $sample = '';
  
  // Handle object properties differently
  if ($item['type'] == 'uri') {
    // Only output a label if it's there
    if (!empty($item['label'])) {
      $sample = '<p class="label">'. $item['label'] .'</p>';
    }
    $sample .= '<em class="uri">'. $item['value'] .'</em>';
  }
  else {
    $sample = '<p class="value">'. $item['value'] .'</p>';
  }
  return $sample;
}

/**
 * uasort() callback for sorting table rows by property name
 */
function rdfimporter_sort_table_rows($a, $b) {
  return strcasecmp($a[0]['data'], $b[0]['data']);
}

