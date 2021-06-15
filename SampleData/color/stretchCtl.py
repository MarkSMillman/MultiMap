import sys
import os, errno
import re

# Main

fh = None;
fileName = sys.argv[1];
levels = [];
levelMap = {};
nv = [];

try:
    fh = open (fileName,'r');
    aline = fh.readline();
    while (aline):
        parts = aline.split();
        if ( parts[0].startswith('nv') ):
            nv = [parts[1],parts[2],parts[3]];
        else:
            levels.append(parts[0]);
            levelMap[parts[0]] = [parts[1],parts[2],parts[3]];
        aline = fh.readline();
        
    parts = parts.sort(reverse=True);
    i = len(levels)-1;
    diff = [];
    while (i > 0):
        level = levels[i];
        next = levels[i-1];
        diff0 = int(levelMap[level][0]); - int(levelMap[next][0]);
        diff1 = int(levelMap[level][1]); - int(levelMap[next][1]);
        diff2 = int(levelMap[level][2]); - int(levelMap[next][2]);
        diff = [diff0,diff1,diff2];
        print str(level)+" "+str(levelMap[level]) + str(diff);
        print str(next)+" "+str(levelMap[next]);
        i = i-1;
        
except IOError:
    print 'IOError';
    
finally:
    fh.close();