"""
"" This DNS server was written by Vladinator (vladix2-gmail-com)
"" to help redirect certain requests, in a sense this is a HOSTS-file
"" compatible with all devices that queries DNS servers.
"""

#!/usr/bin/python
import os, re, socket,sys, thread, time, urllib2
from struct import unpack, pack
started = time.clock()

# declare what domains to redirect to what IP (["domain", "toip"], ...)
# set "toip" to 1 (use local to router IP) or 2 (use public IP) or specify IP or domain
redirect = [["gamestats2.gs.nintendowifi.net", "2"]]

pidf = ".lock"
pid = open(pidf, "w")
pid.writelines("running")
pid.close();

def bye():
  global started, pidf
  os.remove(pidf)
  print "Server has shutdown (ran for %.1f seconds)... bye!"%(time.clock()-started)
  exit(0)

def myip(local=None):
  if local is None:
    con = socket.socket()
    con.connect(("google.com", 80))
    return con.getsockname()[0]
  urls = ["http://folk.uio.no/alexpe/ip", "http://whatismyip.com/automation/n09230945.asp"]
  for url in urls:
    ip = urllib2.urlopen(url).read()
    if re.search(re.compile("^(\d+)\.(\d+)\.(\d+)\.(\d+)$"), ip):
      return ip
  return myip()

def getip(type=None):
  type += ""
  if type is "1":
    return myip()
  elif type is "2":
    return myip(1)
  return type

def dom2ip(domain=None):
  dom2ip = ""
  try:
    dom2ip = socket.gethostbyname_ex(domain)[2][0] # gets the IP of the domain
  except:
    dom2ip = "2" # returns "get public ip" flag if a error occurs
  return dom2ip

def stripdomain(str):
  #return re.sub(re.compile('(\x02|\x03|\x0C)'), ".", str)
  str = re.sub(re.compile('[^a-zA-Z0-9\-\__]'), ".", str)
  for i in range(1, 25):
    str = str.replace("..", ".")
  if str[0:1] == ".":
    str = str[1:]
  if str[len(str)-1:] == ".":
    str = str[0:len(str)-1]
  return str

def getdomain(raw):
  return stripdomain(raw[10:12+raw[12:].find("\x00")])

def isip(ip=None):
  if ip is None:
    return False
  if len(ip.split()) == 1:
    ipl = ip.split(".")
    if len(ipl) == 4:
      for i, item in enumerate(ipl):
        try:
          ipl[i] = int(item)
        except:
          return False
        if not isinstance(ipl[i], int):
          return False
        if max(ipl) < 256:
          return True
        else:
          return False
      else:
        return False
    else:
      return False
  return True

class hijackdns:
  def __init__(self, data):
    self.data = data
    self.domain = ""
    type = (ord(data[2]) >> 3) & 15
    if type == 0:
      pos = 12
      int = ord(data[pos])
      while int != 0:
        self.domain += data[pos+1:pos+int+1]+"."
        pos += int + 1
        int = ord(data[pos])

  def request(self, ip):
    packet = ""
    if self.domain:
      packet += self.data[:2] + "\x81\x80" # id + flags
      packet += self.data[4:6] + self.data[4:6] + "\x00\x00\x00\x00" # questions + answer RRS + authority RRS + additional RRS
      packet += self.data[12:] # query name + query type + query class
      packet += "\xc0\x0c" # answer name (A)
      if isip(ip):
        packet += "\x00\x01\x00\x01\x00\x00\x00\x3c\x00\x04" # answer type (A) + class (IN) + time to live (4sec) + data length
        packet += str.join("", map(lambda x: chr(int(x)), ip.split("."))) # X.X.X.X (internet protocol)
      else:
        answer = "\x01" + "x" + ip # answer primary name (ip is actually a domain name like "docs.google.com")
        packet += "\x00\x05\x00\x01\x00\x00\x00\x04" + pack(">H", len(answer)) # answer type (CNAME) + answer class (IN) + answer time to live (4sec) + data length
        packet += answer
    return packet

def server():
  global redirect
  try:
    con = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    con.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    con.bind(("0.0.0.0", 53))
  except Exception as ex:
    print "failed!"
    print ex
    bye()
  print "done!"
  realdns = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
  try:
    realdns.connect(("8.8.8.8", 53)) # googles dns service
    print "Server is waiting for requests..."
    while 1:
      data, addr = con.recvfrom(1024)
      toip = None
      if len(redirect) > 0:
        for redir in redirect:
          domain = getdomain(data)
          if redir[0] in domain:
            toip = redir[1]
            break
      if toip is not None:
        fakedns = hijackdns(data)
        con.sendto(fakedns.request(toip), addr)
        print "[Hook-%s] \"%s\" -> \"%s\""%(addr[0], domain, toip)
      else:
        realdns.send(data)
        realdata = realdns.recv(512)
        con.sendto(realdata, addr)
        print "[Real-%s] \"%s\" (buypass)"%(addr[0], getdomain(data))
  except Exception as ex:
    print ex
    con.close()
    realdns.close()
    print "Something fatal happened, must shutdown!"
    bye()

try:
  redirect
except Exception as ex:
  redirect = []

if __name__ == "__main__":
  if len(redirect) > 0:
    print "Checking redirect table and fetching proper IP's..."
    try:
      for redir in redirect:
        sys.stdout.write("[%s] \"%s\" set to "%(redir[0], redir[1]))
        redir[1] = getip(redir[1])
        print "\"%s\""%redir[1]
    except Exception as ex:
      print "...\nFatal error: %s"%ex
      bye()
  sys.stdout.write("Starting DNS server in separate thread... ")
  try:
    thread.start_new_thread(server, ())
  except Exception as ex:
    print "failed!"
    print ex
    bye()

while 1:
  try:
    time.sleep(5)
  except KeyboardInterrupt:
    bye()
