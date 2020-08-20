#!/usr/local/bin/python3
import os
import sys
import subprocess


def test_XCEC(exePath, smt='stp', root='./', output='./output/'):
    files = os.listdir(os.path.join(root, 'cases'))
    files.sort(key= lambda x:int(x[4:]))
    if not os.path.exists(os.path.join(root, 'log')):
        os.mkdir(os.path.join(root, 'log'))
    for i, dir in enumerate(files):
        with open(os.path.join(root, 'log/'+dir+'.txt'), 'w', encoding='utf8') as log:
            cmd = [exePath, os.path.join(root, 'cases', dir, 'gf.v'), os.path.join(
                root, 'cases', dir, 'rf.v'), os.path.join(output, 'output_' + dir + '.txt'), smt]
            p = subprocess.Popen(cmd, shell=False, bufsize=0,
                                 stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            while p.poll() is None:
                nextline = p.stdout.readline()
                log.write(nextline.decode())
                log.flush()
            if p.returncode == 0:
                print(dir + ' success!')
            else:
                print(dir + ' failed!')

if __name__ == "__main__":
    smt = 'stp'
    if len(sys.argv) == 2:
        smt = sys.argv[1]
    test_XCEC('../build/XCEC', smt)
