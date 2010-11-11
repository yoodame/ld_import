Requirements
============

- **Drupal 6.x**
- **[Feeds module][feeds]** - tested with version 6.x-1.0-beta10
  - **[Chaos tool suite (CTools) module][ctools]** - Feeds dependency
  - **[Job Scheduler module][jobscheduler]** - Feeds dependency
  - **Feeds UI module** (included with Feeds) - needed for configuring imports, but not required for RDFimporter to function
- **[Libraries module][libraries]** (optional, but recommended) - good way to share ARC2 and other PHP libraries among modules
- **[ARC2 library][arc2]** - open source PHP library for working with RDF

*NOTE: There are issues with some SPARQL endpoints and the way they handle SPARQL DESCRIBE queries, resulting in ARC not getting the data it needs. This has been fixed on ARC's GitHub repository, but is not part of the main release yet. If using RDFimporter with a SPARQL endpoint (especially Sesame), you may want to download the ARC2 library from the [GitHub repository][arc_github] instead.*

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
   
   - If you use the RDF module, or other modules that utilize ARC, it should be available for RDFimporter already.
   - (Using Libraries module) Unpack (or git clone) the ARC2 bundle into either "sites/all/libraries/arc" or "sites/<sitename>/libraries/arc"
   - Unpack or clone the ARC2 bundle directly into "modules/rdfimporter/arc"

   Drupal will show a notice if RDFimporter has trouble finding ARC.

3. Download and enable the RDFimporter module.
  

System settings
---------------
RDFimporter makes a lot of HTTP requests and processes a lot of data. While it does use Drupal's Batch API to help move things along, there are still common issues.

- **PHP maximum execution time:**
  The default seems to be around 30 seconds, which is sometimes too low for even Drupal itself. This is set with "max_execution_time" in "php.ini"

- **PHP memory limit:**
  Unless you're importing a whole lot of data this may never be an issue, but it's something to be aware of. It is set with "memory_limit" in "php.ini"

- **MySQL packet size limit:**
  The Drupal Batch API temporarily saves data to the database between each batch that it processes. In the case of RDFimporter, this means saving a big chunk of data that it fetched. The default limit for some installations is 1MB, which is a definite problem. This is set with "max_allowed_packet" in "my.cnf". The [Drupal requirements page][requirements] recommends at least 16MB.

[requirements]: http://drupal.org/requirements

Changing any these settings requires system admin privileges. If you're working on a server where you don't have privileges, the [Drupal Tweaks module][tweaks] is worth a shot.

[tweaks]: http://drupal.org/project/drupal_tweaks


Usage
=====
Coming soon...
