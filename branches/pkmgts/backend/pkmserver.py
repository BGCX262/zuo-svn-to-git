"""
"" This PKM "server" was written by Vladinator (vladix2-gmail-com)
"" to act as a distrobution system until user terminates the script.
"""

#!/usr/bin/python
import sys, itertools, struct, os.path
import time, string, re, hashlib
from os import curdir, sep
from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
started = time.clock()
sessionvars = {}
port = 80 # default 80 (to work for the NDS)

# comment out line below to disable distrobution or point it to a PKM/BIN file
distro = "./pkmfiles/[Vlad] Arceus.pkm"

def run(program, *args, **kw):
  mode = kw.get("mode", os.P_WAIT)
  for path in string.split(os.environ["PATH"], os.pathsep):
    file = os.path.join(path, program) + ".exe"
    try:
      return os.spawnv(mode, file, (file,) + args)
    except Exception as ex:
      return ""

def getrawpkm(filepath):
  try:
    os.system("python \"%s\" \"%s\" 1"%("pkmlib.py", filepath))
    raw = ""
    filepathproper = "%s.proper"%filepath
    try:
      f = open(filepathproper, "rb")
      raw = f.read()
      f.close()
      try:
        os.remove(filepathproper)
      except Exception as ex:
        print "Warning: could not delete \"%s\"."%filepathproper
    except Exception as ex:
      raw = "\x01\x00"
    return raw
  except Exception as ex:
    return "\x01\x00"

try:
  distro
except Exception as ex:
  distro = ""

def getquery(uri):
  query = {}
  matches = re.split("(\?|&)(\w+)=", uri)
  if len(matches) > 1:
    for i in range(2, len(matches), 3):
      query[matches[i]] = matches[i+1]
  return query

def getfile(uri):
  matches = re.split("\/(.*?)(\?|&)", uri)
  if len(matches) > 1:
    return matches[1]
  matches = re.split("\/(.*?)$", uri)
  return matches[1][0:11] == "pokemondpds"

def exists(lst, obj):
  try:
    lst[obj]
    return True
  except Exception as ex:
    return False

def getresponse(addr, path):
  global sessionvars, distro
  out = ""
  query = getquery(path)
  querylen = len(query)
  queryfile = getfile(path)
  if queryfile == True: # is a subfile inside /pokemondpds/ but dunno what thus show ACK just in case
    out += "\x01\x00"
  elif querylen == 0: # no query means a hello world message -yay!
    out += "Hello world!"
  elif querylen == 1: # one query (pid) generate a hash to return
    sha = hashlib.sha1()
    sha.update(query["pid"])
    out += sha.hexdigest()[:32]
    sha = hashlib.sha1()
    sha.update("sAdeqWo3voLeC5r16DYv%s"%out)
    expecthash = sha.hexdigest()
  else: # expects a page request...
    queryfile = queryfile.lower()
    if queryfile.endswith("worldexchange/info.asp"):
      out += "\x01\x00"
    elif queryfile.endswith("common/setprofile.asp"):
      out += "\x00"*8
    elif queryfile.endswith("worldexchange/result.asp"):
      if len(distro) > 0:
        if exists(sessionvars, addr) == False:
          sessionvars[addr] = {}
          sessionvars[addr]["issent"] = False
        if sessionvars[addr]["issent"] == False:
          out += getrawpkm(distro)
        else:
          out += "\x04\x00"
      else:
        out += "\x05\x00"
    elif queryfile.endswith("worldexchange/get.asp"):
      if len(distro) > 0:
        out += getrawpkm(distro)
      else:
        out += "\x01\x00"
    elif queryfile.endswith("worldexchange/info.asp") or queryfile.endswith("worldexchange/delete.asp"):
      out += "\x01\x00"
      if len(distro) > 0:
        if exists(sessionvars, addr) == False:
          sessionvars[addr] = {}
          sessionvars[addr]["issent"] = False
        else:
          sessionvars[addr]["issent"] = True
    else:
      out += "\x01\x00"
  return out

class httphandler(BaseHTTPRequestHandler):
  def do_GET(self):
    try:
      html = ""
      try:
        html += getresponse(self.client_address[0], self.path)
      except Exception as ex:
        print ex
      if html is not "":
        self.send_response(200)
        localtime = time.localtime(time.time())
        days = ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"]
        months = ["Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"]
        datestamp = "%s, %d %s %d %d:%d:%d GMT"%(days[localtime[6]], localtime[2], months[localtime[1]-1], localtime[0], localtime[3], localtime[4], localtime[5])
        self.send_header("Date", datestamp)
        self.send_header("Server", "Microsoft-IIS/6.0")
        self.send_header("P3P", "CP='NOI ADMa OUR STP'")
        self.send_header("Cluster-server", "aphexweb3")
        self.send_header("X-Server-Name", "AW4")
        self.send_header("X-Powered-By", "ASP.NET")
        self.send_header("Content-Length", len(html))
        self.send_header("Content-Type", "text/html")
        self.send_header("Set-Cookie", "ASPSESSIONIDQCDBDDQS=JFDOAMPAGACBDMLNLFBCCNCI; path=/")
        self.send_header("Cache-control", "private")
        self.end_headers()
        self.wfile.write(html)
      else:
        self.send_response(404)
        self.send_header("Content-Type", "text/html")
        self.end_headers()
        self.wfile.write("404 - File not found.")
      return
    except Exception as ex:
      print ex
      pass

if __name__ == "__main__":
  try:
    print "Started HTTP server on port %d..."%port
    srv = HTTPServer(("", port), httphandler)
    srv.serve_forever()
  except KeyboardInterrupt:
    srv.socket.close()
    print "Crtl+C pressed. Shutting down."
    exit(0)
