**Unfolder** is an automated tool for discovering hidden files and directories
hosted on HTTP servers. Given a base URL and a dictionary, it scans the HTTP
server for URLs that return non-404 status codes.

#### Installation
Download unfolder by cloning the Git repository:
~~~ bash
git clone https://github.com/SuprDewd/unfolder.git
~~~

Build unfolder using autotools:
~~~ bash
./autogen.sh
./configure
make
sudo make install
~~~

#### Usage

Choose one or more of the following dictionaries of common file and directory names:
- https://github.com/danielmiessler/SecLists
- https://github.com/v0re/dirb/tree/master/wordlists

Run unfolder as follows:
~~~ bash
unfolder -u http://example.com/base_url/ -d dict1.txt -d dict2.txt
~~~

#### TODO
- Support HTTPS connections
- Recover when server closes keep-alive connection
- Interactive CLI
    - Ability to add new base URLs and dictionaries at run-time

