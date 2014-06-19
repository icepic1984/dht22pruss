LIBPATH=-L ~/prusslib/lib/
INCLUDEPATH=-I ~/prusslib/include/
LIBS+=-lprussdrv
PASM=~/prusslib/pasm
EXECUTABLE=prudht22

all: $(EXECUTABLE)  $(EXECUTABLE).bin $(EXECUTABLE).o 
%.o: %.cpp
	g++ -c $(INCLUDEPATH) $< 

% : %.o
	g++  $(LIBPATH)  $(LIBS) $< -o $@

%.bin : %.p
	$(PASM) -b $<

clean: 
	-rm -rf $(EXECUTABLE) $(EXECUTABLE).o $(EXECUTABLE).bin
