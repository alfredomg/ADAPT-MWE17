all: context-features-train context-features-test print-annotated-expressions
#all: context-main test context-features-train context-features-test

print-annotated-expressions:  print-annotated-expressions.cpp ContextCollection.o annotatedcorpus.o annotatedsentence.o outcomes.o Context.o misc.o Sentence.o Corpus.o
	g++ -o print-annotated-expressions -std=c++11 ContextCollection.o annotatedcorpus.o annotatedsentence.o outcomes.o Context.o misc.o Sentence.o Corpus.o print-annotated-expressions.cpp
context-features-train: context-features-train.cpp ContextCollection.o annotatedcorpus.o annotatedsentence.o outcomes.o Context.o misc.o Sentence.o Corpus.o
	g++ -o context-features-train -std=c++11 ContextCollection.o annotatedcorpus.o annotatedsentence.o outcomes.o Context.o misc.o Sentence.o Corpus.o context-features-train.cpp
context-features-test: context-features-test.cpp ContextCollection.o annotatedcorpus.o annotatedsentence.o outcomes.o Context.o misc.o Sentence.o Corpus.o
	g++ -o context-features-test -std=c++11 ContextCollection.o annotatedcorpus.o annotatedsentence.o outcomes.o Context.o misc.o Sentence.o Corpus.o context-features-test.cpp
#context-main:  context-features-test.cpp ContextCollection.o annotatedcorpus.o annotatedsentence.o outcomes.o Context.o misc.o Sentence.o Corpus.o context-main.cpp
#	g++ -o context-main -std=c++11  context-features-test.cpp ContextCollection.o annotatedcorpus.o annotatedsentence.o outcomes.o Context.o misc.o Sentence.o Corpus.o context-main.cpp
#test:  Sentence.o Corpus.o Context.o annotatedcorpus.o  annotatedsentence.o outcomes.o  misc.o test.cpp
#	g++ -o test -std=c++11 Sentence.o Corpus.o Context.o  annotatedcorpus.o  annotatedsentence.o outcomes.o misc.o test.cpp
Sentence.o: Sentence.h Sentence.cpp
	g++ -std=c++11 -c Sentence.cpp
Corpus.o: Corpus.h Corpus.cpp Sentence.h
	g++ -std=c++11 -c Corpus.cpp
Context.o: Context.h Context.cpp Corpus.h Sentence.h
	g++ -std=c++11 -c Context.cpp
misc.o: misc.cpp Corpus.h Context.h misc.h annotatedcorpus.h ContextCollection.h
	g++ -std=c++11 -c misc.cpp
annotatedcorpus.o: annotatedcorpus.h annotatedcorpus.cpp annotatedsentence.h outcomes.h
	g++ -std=c++11 -c annotatedcorpus.cpp
annotatedsentence.o: annotatedsentence.cpp annotatedsentence.h outcomes.h
	g++ -std=c++11 -c annotatedsentence.cpp
outcomes.o: outcomes.cpp outcomes.h  misc.h
	g++ -std=c++11 -c outcomes.cpp
ContextCollection.o: ContextCollection.h ContextCollection.cpp annotatedcorpus.h annotatedsentence.h outcomes.h Context.h misc.h
	g++ -std=c++11 -c ContextCollection.cpp

clean: 
	rm -f *.o test context-main context-features-train context-features-test
