include ../../Makefile.common

OBJS = \
    PaintableWidget.o \
    PaletteWidget.o \
    ToolboxWidget.o \
    Tool.o \
    PenTool.o \
    BucketTool.o \
    ColorDialog.o \
    SprayTool.o \
    main.o

APP = PaintBrush

DEFINES += -DUSERLAND

all: $(APP)

$(APP): $(OBJS)
	$(LD) -o $(APP) $(LDFLAGS) $(OBJS) -lgui -lcore -lc

.cpp.o:
	@echo "CXX $<"; $(CXX) $(CXXFLAGS) -o $@ -c $<

-include $(OBJS:%.o=%.d)

clean:
	@echo "CLEAN"; rm -f $(APP) $(OBJS) *.d

