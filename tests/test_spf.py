#!/usr/bin/env python

import os,sys

tests=[]
test=["","","","",""]
for line in open(os.environ["TDIR"]+"/spftest.txt"):
        if line[0]=="#":
                continue
        if len(line)==1:
                tests.append(test)
                test=["","","","",""]

        if line.startswith("spfquery "):
                pos1=line.find("-ip=")
                pos2=line.find(" ",pos1)
                test[0]=line[pos1+4:pos2]

                pos1=line.find("-sender=")
                pos2=line.find(" ",pos1)
                test[1]=line[pos1+8:pos2]

                pos1=line.find("-helo=")
                pos2=line.find(" ",pos1)
                test[2]=line[pos1+6:pos2]


        if line.startswith("result"):
                pos1=line.rfind(" ")
                test[4]=line[pos1+1:-1].upper()

failures=0
successes=0
counter=1

for test in tests:

        if test == ["","","","",""]:
                continue

        print "Test nr. ",counter

        os.environ ["TCPREMOTEIP"]=test[0]
        os.environ ["SMTPMAILFROM"]=test[1]
        os.environ ["HELOHOST"]=test[2]
        os.environ ["SMTPRCPTTO"]=""

        p=os.popen("./spfchecks 2>&1","r")
        res=p.read()
        ex=p.close()
        if ex:  # Probably a segfault
                print test
                print res
                sys.exit(0)

        if "SPF_RESULT_"+test[4] in res:
                print "OK"
                successes+=1
        else:
                print "ERROR"
                print test
                print res
                failures+=1

        sys.stdout.flush()
        counter=counter+1


if failures > 0:
        print "TEST NOT SUCCESSFUL"
else:
        print "TEST SUCCESSFUL"
print
print "Successes: ",successes
print "Failures:  ",failures
