NSYM	= 1000
NVERTEX	= 100
MAXSUCC	= 4
NACTIVE	= 100
PRINT   = 1

all:
	@mkdir -p classes
	#scalac -g:vars -d classes dataflow.scala
	#scalac -deprecation -d classes dataflow.scala
	~/scala-2.10.6/bin/scalac -deprecation -optimise -d classes dataflow.scala
	~/scala-2.10.6/bin/scala -classpath classes Driver $(NSYM) $(NVERTEX) $(MAXSUCC) $(NACTIVE) $(PRINT)

run:
	~/scala-2.10.6/bin/scala -classpath classes Driver $(NSYM) $(NVERTEX) $(MAXSUCC) $(NACTIVE) $(PRINT)


asdf:
	@number=1 ; while [[ $$number -le 10 ]] ; do \
        	echo $$number ; \
        	((number = number + 1)) ; \
    	done

qwer:
	for number in 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1  ; do \
		~/scala-2.10.6/bin/scala -classpath classes Driver $(NSYM) $(NVERTEX) $(MAXSUCC) $(NACTIVE) $(PRINT) ; \
	done
