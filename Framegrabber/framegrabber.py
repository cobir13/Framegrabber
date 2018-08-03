import zmq
import subprocess
import re

default_hostname = "tcp://localhost:5555"

class FramegrabberApp(object):
    def __init__(self, id):
        self.id = id

class Framegrabber(object):
    FGrunning = False
    match_return = r'\s*(\w*)\s*(\w*)\s*\(([^)]*)\);\s*'
    def __init__(self, hostname=default_hostname, timeout=1000):
        self.context = zmq.Context()
        print("Connecting to {}".format(hostname))
        self.socket = self.context.socket(zmq.REQ)
        self.socket.RCVTIMEO = timeout
        self.socket.connect(hostname)
        self.socket.setsockopt(zmq.LINGER, timeout)
        print("Connected to {}".format(hostname))
    
    @staticmethod
    def start_framegrabber():
        if Framegrabber.FGrunning:
            raise RuntimeWarning("Only one instance of Framegrabber.exe can run at once")
        Framegrabber.proc = subprocess.Popen(
            ["C:/Users/Keck Project/Documents/Framegrabber/Release/Framegrabber.exe"],
            stdout=subprocess.PIPE
        )
        FramegrabberApp.FGrunning = True


    # Base methods (wrapper around Framegrabber Command Language)
    
    def send(self, msg):
        self.socket.send(msg.encode('utf-8'))
        try:
            reply = self.socket.recv.decode('utf-8')
        except:
            print("Timeout")
            raise TimeoutError("Send timed out")
        
        return reply
    
    def start_app(self, appname, params=""):
        appcmd = Framegrabber.build_command("STREAM", appname, params)
        reply = self.send(appcmd)
        reply_parts = Framegrabber.break_return(reply)
        if reply_parts[0] == "SUCCESS":
            print("Created {} (id {})".format(reply_parts[1], reply_parts[2]))
            return FramegrabberApp(reply_parts[2])
        elif reply_parts[0] == "ERROR":
            raise RuntimeError("Error creating framegrabber app (internal error {}:\"{}\"".format(reply_parts[1], reply_parts[2]))
        else:
            raise RuntimeError("Unknown return code")
    
    def kill_app(self, app):
        appcmd = Framegrabber.build_command("KILLAPP", app.id)
        reply = self.send(appcmd)
        reply_parts = Framegrabber.break_return(reply)
        if reply_parts[0] == "SUCCESS":
            print("Killed {} (if it existed)".format(reply_parts[1]))
            return True
        elif reply_parts[0] == "ERROR":
            raise RuntimeError("Error killing framegrabber app (internal error {}:\"{}\"".format(reply_parts[1], reply_parts[2]))
        else:
            raise RuntimeError("Unknown return code")
    
    def write_word(self, word, val):
        appcmd = Framegrabber.build_command("STREAM", word, val)
        reply = self.send(appcmd)
        reply_parts = Framegrabber.break_return(reply)
        if reply_parts[0] == "SUCCESS":
            print("Wrote {}".format(reply_parts[1]))
            return True
        elif reply_parts[0] == "ERROR":
            raise RuntimeError("Error writing serial words (internal error {}:\"{}\"".format(reply_parts[1], reply_parts[2]))
        else:
            raise RuntimeError("Unknown return code")
    
    def get_word(self, word):
        if (word not in ("wax", "way", "waxy", "tint")):
            raise RuntimeWarning("Unknown serial word to read.")
        
        appcmd = Framegrabber.build_command("STREAM", word)
        reply = self.send(appcmd)
        reply_parts = Framegrabber.break_return(reply)
        if reply_parts[0] == "SUCCESS":
            return reply_parts[2]
        
        elif reply_parts[0] == "ERROR":
            raise RuntimeError("Error reading serial words (internal error {}:\"{}\"".format(reply_parts[1], reply_parts[2]))

        else:
            raise RuntimeError("Unknown return code")
    
    def quit(self):
        try:
            self.send("QUIT;")
        except TimeoutError:
            return

    # Static utility methods
    
    @staticmethod
    def break_return(returnval):
        match = re.fullmatch(Framegrabber.match_return, returnval)
        if match:
            return match.groups()
        return None
    
    @staticmethod
    def build_command(base, subcommand, args=""):
        return "{} {}({});".format(base, subcommand, args)
    
    # API for use:

    def FullFrame(self, numframes, saveloc):
        return self.start_app("fullframe", '{}, "{}"'.format(numframes, saveloc))
    
    def Focuser(self, numframes, saveloc, x, y):
        return self.start_app("focuser", '{}, "{}", {}, {}'.format(numframes, saveloc, x, y))
    
    def Window(self):
        return self.start_app("window")
    
    def SetWAX(self, val):
        self.write_word("wax", val)
    
    def SetWAY(self, val):
        self.write_word("way", val)
    
    def SetWAXY(self, waxy):
        self.SetWAX(waxy[0])
        self.SetWAY(waxy[1])
    
    def SetTint(self, tint):
        self.write_word("tint", tint)
    
    def GetWAXY(self):
        str_waxy = self.get_word("waxy")
        match = re.fullmatch(r'\s*\[(\d*), (\d*)\]\s*', str_waxy)
        if match:
            groups = match.groups()
            return (int(groups[0]), int(groups[1]))
        raise RuntimeError("Could not parse WAXY ({})".format(str_waxy))
    
    def GetTint(self):
        str_tint = self.get_word("tint")
        return int(str_tint)
        