== Important information ==
1. The site is by default located in the webserver root as:
/pokemon/gts/

Note: If you do not keep the same structure you must remember to edit
the .htaccess (2nd line, "RewriteBase /pokemon/gts/").

2. Edit the config.php, most important lines to edit are:
14-17 for the MySQL settings
20 for the site url
30 if you do not run the DNS on the same server as the website
32 if you edit line 30 you may need to edit this one too

== Where things are and how it works ==
The DNS server is located at /pokemon/gts/backend/ as "dnsserver.py"
For Windows users it's best if you run the "encode_pkm.bat" when you
add or make changes to the "pkmfiles" dir. But if you remove or add
pokemon you must edit the MySQL table "events".

The "db_gts.sql" contains the MySQL tables, by default there are some
active events that are valid and working.

The dnsserver.py must run for the website to work, if not then the site
will be in a "sleep" mode until the server is back online. For now I've
tested this only with a local DNS server, running as is directly from
the backend folder but in theory it could run from anywhere -only need
to perhaps tweak the PHP code for more efficiency.

== Requires ==
Port TCP80 for webserver and UDP53 for DNS.
The UDP53 is used by the webserver for status checks. The DNS server
uses this to handle incoming requests. More help about this can be
found by using Google and searching for your router and how you forward
ports properly. Note if you host on a remote server then they better
have the ports open or it won't work.

Python 2.6 (not tested with 3.X series, don't know if it works)
MySQL5 or higher (I doubt many run version 4 anymore)
PHP5 (again, doubt anyone runs version 4 anymore)
