LIBPATH=-L ~/prusslib/lib/
INCLUDEPATH=-I ~/prusslib/include/
LIBS+=-lprussdrv -lpthread
PASM=~/prusslib/pasmc
EXECUTABLE=prudht22
DEVICEOVERLAY=DM-GPIO-PRU
OBJECTS=prudht22.o bbbdht22.o


all: $(EXECUTABLE) $(OBJECTS) $(EXECUTABLE).bin  $(DEVICEOVERLAY).dtbo

$(EXECUTABLE) : $(OBJECTS) 
	g++  $(LIBPATH)  $(LIBS) $+ -o $@

%.o: %.cpp
	g++ -std=c++11 -c $(INCLUDEPATH) $< 


%.bin : %.p
	$(PASM) -b $<

%.dtbo: %.dts
	dtc -O dtb -o $(basename $@)-00A0.dtbo -b 0 -@ $<
	cp $(basename $@)-00A0.dtbo /lib/firmware

clean: 
	-rm -rf $(EXECUTABLE) $(OBJECTS) $(EXECUTABLE).bin
