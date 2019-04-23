MACHINE_TYPE ?= macosx
SRIPATH ?= /Users/VincentLin/Desktop/dsp_hw3/srilm-1.5.10
LM ?= bigram.lm


CXX = g++
CXXFLAG = -O3 -I$(SRIPATH)/include -w --std=c++11
TARGET = mydisambig
SRC = mydisambig.cpp
OBJ = $(SRC:.cpp=.o)

MAP_IN = Big5-ZhuYin.map
MAP_OUT = ZhuYin-Big5.map 
RM_FILES =  $(OBJ) $(TARGET)

vpath lib%.a $(SRIPATH)/lib/$(MACHINE_TYPE)

.PHONY: all clean

all: $(TARGET)

$(TARGET):$(OBJ) -loolm -ldstruct -lmisc
	$(CXX) $(LDFLAGS) -o $@ $^

%.o:%.cpp
	$(CXX) $(CXXFLAG) -c $<

run:
	mkdir -p result2
	@for i in $(shell seq 1 10) ; do \
		echo "Running bigram $$i.txt"; \
		 ./mydisambig -text testdata/$$i.txt -map $(MAP_OUT) -lm $(LM) -order 2 > result2/$$i.txt; \
	done;

run_trigram:
	mkdir -p result3
	@for i in $(shell seq 1 10) ; do \
		echo "Running trigram $$i.txt"; \
		 ./mydisambig -text testdata/$$i.txt -map $(MAP_OUT) -lm $(LM) -order 3 > result3/$$i.txt; \
	done;


run_srilm:
	mkdir -p result1
	@for i in $(shell seq 1 10) ; do \
		echo "Running $$i.txt"; \
		$(SRIPATH)/bin/$(MACHINE_TYPE)/disambig -text testdata/$$i.txt -map $(MAP_OUT) -lm $(LM) -order 2 > result1/$$i.txt; \
	done;

run_test:
	./separator_big5.pl corpus.txt > corpus_seg.txt
	./separator_big5.pl testdata/1.txt > 1_seg.txt
	$(SRIPATH)/bin/$(MACHINE_TYPE)/ngram-count -text corpus_seg.txt -write lm.cnt -order 2
	$(SRIPATH)/bin/$(MACHINE_TYPE)/ngram-count -read lm.cnt -lm bigram.lm  -unk -order 2
	
map: mapping.py
	python3 mapping.py $(MAP_IN) $(MAP_OUT)

clean:
	rm $(RM_FILES)
