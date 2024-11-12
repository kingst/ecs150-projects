all: gunrock_web mkfs ds3ls ds3cat ds3bits ds3mkdir ds3cp ds3touch ds3rm

CC = g++
CFLAGS = -g -Werror -Wall -I include -I shared/include -fsanitize=address
LDFLAGS = -pthread
VPATH = shared

OBJS = gunrock.o MyServerSocket.o MySocket.o HTTPRequest.o HTTPResponse.o http_parser.o HTTP.o HttpService.o HttpUtils.o FileService.o dthread.o WwwFormEncodedDict.o StringUtils.o Base64.o HttpClient.o HTTPClientResponse.o DistributedFileSystemService.o LocalFileSystem.o Disk.o

DSUTIL_OBJS = Disk.o LocalFileSystem.o StringUtils.o

-include $(OBJS:.o=.d)

gunrock_web: $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(OBJS) $(LDFLAGS)

mkfs: mkfs.o
	gcc -o $@ $(CFLAGS) mkfs.o

ds3ls: ds3ls.o $(DSUTIL_OBJS)
	$(CC) -o $@ $(CFLAGS) ds3ls.o $(DSUTIL_OBJS)

ds3cp: ds3cp.o $(DSUTIL_OBJS)
	$(CC) -o $@ $(CFLAGS) ds3cp.o $(DSUTIL_OBJS)

ds3cat: ds3cat.o $(DSUTIL_OBJS)
	$(CC) -o $@ $(CFLAGS) ds3cat.o $(DSUTIL_OBJS)

ds3rm: ds3rm.o $(DSUTIL_OBJS)
	$(CC) -o $@ $(CFLAGS) ds3rm.o $(DSUTIL_OBJS)

ds3bits: ds3bits.o $(DSUTIL_OBJS)
	$(CC) -o $@ $(CFLAGS) ds3bits.o $(DSUTIL_OBJS)

ds3mkdir: ds3mkdir.o $(DSUTIL_OBJS)
	$(CC) -o $@ $(CFLAGS) ds3mkdir.o $(DSUTIL_OBJS)

ds3touch: ds3touch.o $(DSUTIL_OBJS)
	$(CC) -o $@ $(CFLAGS) ds3touch.o $(DSUTIL_OBJS)

%.d: %.c
	@set -e; gcc -MM $(CFLAGS) $< \
		| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@;
	@[ -s $@ ] || rm -f $@

%.d: %.cpp
	@set -e; $(CC) -MM $(CFLAGS) $< \
		| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@;
	@[ -s $@ ] || rm -f $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

clean:
	rm -f gunrock_web mkfs ds3ls ds3cat ds3bits ds3cp ds3mkdir ds3touch ds3rm *.o *~ core.* *.d
