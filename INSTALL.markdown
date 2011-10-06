Requirements
============

- **Drupal 7.x**
- **[Feeds module][feeds]** - tested with version 7.x-2.0-alpha4
  - **[Chaos tool suite (CTools) module][ctools]** - Feeds dependency
  - **[Job Scheduler module][jobscheduler]** - Feeds dependency
  - **Feeds UI module** (included with Feeds) - needed for configuring imports, but not required for Linked Data Import to function
- **[Libraries module][libraries]** (optional, but recommended) - good way to share ARC2 and other PHP libraries among modules
- **[ARC2 library][arc2]** - open source PHP library for working with RDF

*NOTE: There are issues with some SPARQL endpoints and the way they handle SPARQL DESCRIBE queries, resulting in ARC not getting the data it needs. This has been fixed on ARC's GitHub repository, but is not part of the main release yet. If using Linked Data Import with a SPARQL endpoint (especially Sesame), you may want to download the ARC2 library from the [GitHub repository][arc_github] instead.*

[feeds]: http://drupal.org/project/feeds
[ctools]: http://drupal.org/project/ctools
[jobscheduler]: http://drupal.org/project/job_scheduler
[libraries]: http://drupal.org/project/libraries
[arc2]: http://arc.semsol.org/download
[arc_github]: https://github.com/semsol/arc2


Installation
============
1. Download and enable the following Drupal modules: ctools, job_scheduler, feeds, feeds_ui

2. Install the ARC2 library. 

   There are a few different ways to do this...
   
   - If you use the RDFx module, or other modules that utilize ARC, it should be available to Linked Data Import already.
   - (Using Libraries module) Unpack (or git clone) the ARC2 bundle into either "sites/all/libraries/arc" or "sites/<sitename>/libraries/arc"
   - Unpack or clone the ARC2 bundle directly into "modules/ld_import/arc"

   Drupal will show a notice if Linked Data Import has trouble finding ARC.

3. Download and enable the Linked Data Import module.
  

System settings
---------------
Linked Data Import makes a lot of HTTP requests and retrieves a lot of data. While it does use Drupal's Batch API to help move things along smoothly, there are still common issues.

- **PHP maximum execution time:**
  The default seems to be around 30 seconds, which is sometimes too low for even Drupal itself. This is set with "max_execution_time" in "php.ini". A good figure for development might be 180 seconds.

- **PHP memory limit:**
  Unless you're importing a whole lot of data this may never be an issue, but it's something to be aware of. It is set with "memory_limit" in "php.ini"

- **MySQL packet size limit:**
  The Drupal Batch API temporarily saves data to the database between each batch that it processes. In the case of Linked Data Import, this means saving a big chunk of data that it fetched. The default limit for some installations is 1MB, which is a definite problem. This is set with "max_allowed_packet" in "my.cnf". The [Drupal requirements page][requirements] recommends at least 16MB.

[requirements]: http://drupal.org/requirements

Changing any these settings requires system admin privileges. If you're working on a server where you don't have privileges, the [Drupal Tweaks module][tweaks] is worth a shot.

[tweaks]: http://drupal.org/project/drupal_tweaks


Basic usage
===========
Setting up imports involves creating *Feeds importers* using the Feeds UI (separate module packaged with Feeds).

First, you need:

- (If importing Linked Data via content negotiation) A list of Linked Data URIs
- (If using a SPARQL endpoint) A list of URIs from an SPARQL endpoint **OR** a SPARQL SELECT query that returns a list of URIs from the endpoint
- A content type configured with fields to store data

To create your first Linked Data importer...

1. Go to "/admin/build/feeds" and click "New importer".

2. Give it a name and description.

3. On the next screen you should see four sections: Basic settings, Fetcher, Parser, Processor. We'll go through these one by one...
   
   1. **Basic settings** 
   
      Click the "Settings" link
      - *Attach to content type:* choose "Use standalone form"
      - *Minimum refresh period:* for testing, choose "Never"
      - *Import on submission:* leave this on when using "Use standalone form"
      
   2. **Fetcher**
   
      Change to either "Linked Data Import Simple Fetcher" or "Linked Data Import SPARQL Fetcher" depending on your data source.
   
   3. **Parser**
   
      Change to "Linked Data Import Parser"
   
   4. **Processor**
      
      Change to "Linked Data Import Node Processor"
   
   From this point you can ignore the first 3 sections because all the settings you'll be changing are in the "Processor" section.
   
4. Go to "Settings" under the "Processor" section.

   - *Content type:* select the type of nodes you want to produce
   - *Input format:* select the input format you want to use (only for the node's body field)
   - *Author:* self-explanatory
   - *Expire nodes:* this could be useful for time-sensitive data
   - *Update existing nodes:* also self-explanatory
   
5. Before setting up any mappings we need to run a *preview* of the data to see properties available for mapping. Go to "/import" where you should see the importer you just created. Click on the importer. If it's not there, you probably attached the importer to a content type, which means will want to create a new node of that type. 

6. Paste in your list of URIs or SPARQL query. Leave the preview options on their default settings for now (but if your URI list is huge, try setting the item limit around 20 or 50). Click "Save and Preview".

7. Once the preview is finished, you should see a table full of sample data -- if you don't see any sample data, there may be a problem with the URIs or SPARQL query you entered. The properties listed here are all the RDF properties being used among the URIs provided. 

8. Go to the "Edit mappings" link just above the preview table. You can also go to "/admin/build/feeds/<importer_name>/mapping".

9. The mappings screen is where you define how RDF property data gets mapped to Drupal nodes. One node will be created/updated for every URI you provided. 

10. When finished setting up mappings, go back to "/import" and then to your importer.

11. Click the "Import" button at the bottom to start the process. When finished you should see a Drupal message telling you that nodes have been created.

