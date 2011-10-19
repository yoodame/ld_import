<?php

/**
 * @file
 * Home of the LinkedDataImportParser.
 */
 
// Make sure the ARC2 library is available.
ld_import_include_arc();
module_load_include('inc', 'ld_import', 'ld_import');

/**
 * Class definition for LinkedDataImportParser.
 */
class LinkedDataImportParser extends FeedsParser {
  
  protected $language;

  /**
   * Implements FeedsParser::parse().
   */
  public function parse(FeedsSource $source, FeedsFetcherResult $fetcher_result) {
    $this->setLanguage($source);

    // This ARC2_Resource instance gets reused throughout this page load.
    $arc = $source->importer->fetcher->getArc($source);

    $list = $fetcher_result->getList();
    $state = $source->state(FEEDS_PARSE);
    $state->total = count($list);
    
    $start = $state->pointer ? $state->pointer : 0;
    $limit = $source->importer->getLimit();
    
    $items = array();
    
    // Loop through URIs until we hit the limit or finish the list.
    for ($i = $start; $i < $state->total && count($items) < $limit; $i++) {
      
      $item = new LinkedDataImportResource($list[$i], $arc, $this->language);
      
      // Only add this item for processing if it returns data.
      if ($item->hasData()) {
        $items[] = $item;
        $state->created++;
      }
      else {
        $state->failed++;
      }
      // Update progress and pointer.
      $state->progress($state->total, $state->created + $state->failed);
      $state->pointer = $i + 1;
    }
    return new FeedsParserResult($items);
  }
  
  /**
   * Implements FeedsParser::getMappingSources().
   */
  public function getMappingSources() {
    return FALSE;
  }
  
  /**
   * Implements FeedsParser::getSourceElement().
   */
  public function getSourceElement(FeedsSource $source, FeedsParserResult $result, $element_key) {
    $resource = $result->currentItem();
    
    if ($element_key == 'URI') {
      return $resource->getUri();
    }
    else if ($element_key == 'Label') {
      return $resource->getLabel();
    }
    else if ($predicates = explode(' [] ', $element_key)) {
      $values = $this->extractPropertyValues($resource, $predicates);
      return $values;
    }
    else {
      return array();
    }
  }
  
  /**
   * Retrieve source data based on predicates in each mapping source.
   */
  public function extractPropertyValues(LinkedDataImportResource $resource, $predicates) {
    $values = array();
      
    $predicate = current($predicates);
    
    // Only proceed if there is data for this predicate.
    if ($property_values = $resource->getProp($predicate)) {
      
      $next_predicate = next($predicates);
      
      foreach ($property_values as $item) {
        if ($next_predicate && $item['type'] == 'uri') {
          $next_resource = new LinkedDataImportResource($item['value'], $resource->getArc(), $this->language);
          $values = array_merge($values, $this->extractPropertyValues($next_resource, $predicates));
        }
        else if (!$next_predicate) {
          $values[] = $item['value'];
        }
      }
    }
    return $values;
  }
  
  // @todo make configurable
  public function setLanguage(FeedsSource $source) {
    global $language;
    $this->language = $language->language;
  }
}
