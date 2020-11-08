###############STEP 1###############

FROM alpine:3.8 as builder_packager

# Install packages needed for Shaka Packager.

RUN apk add --no-cache bash build-base curl findutils git ninja python \
                       bsd-compat-headers linux-headers libexecinfo-dev

# Install depot_tools.
RUN git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
ENV PATH $PATH:/depot_tools
ENV GCLIENT_PY3=0

# Alpine uses musl which does not have mallinfo defined in malloc.h. Define the
# structure to workaround a Chromium base bug.
RUN sed -i \
    '/malloc_usable_size/a \\nstruct mallinfo {\n  int arena;\n  int hblkhd;\n  int uordblks;\n};' \
    /usr/include/malloc.h

ENV GYP_DEFINES='clang=0 use_experimental_allocator_shim=0 use_allocator=none musl=1'
ENV GCLIENT_PY3=0

# Build shaka-packager
RUN /depot_tools/gclient.py config https://github.com/masantiago/shaka-packager.git --name=src --unmanaged
RUN /depot_tools/gclient.py sync --with_branch_heads --with_tags
RUN cd /src && git checkout tcs-shaka-2.4.3 && cd -
RUN gclient sync --with_branch_heads --with_tags
RUN ninja -C /src/out/Release
RUN ls -lsha /src/out/Release

###############STEP 2###############

FROM alpine:3.8

RUN apk add --no-cache libstdc++ python

# Copy our executable
COPY --from=builder_packager /src/out/Release/packager \
                    /src/out/Release/mpd_generator \
                    /src/out/Release/pssh-box.py \
                    /usr/bin/
COPY --from=builder_packager /src/out/Release/pyproto /usr/bin/pyproto
