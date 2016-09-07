struct ccnx_codec_network_buffer_iovec {
    CCNxCodecNetworkBuffer *networkBuffer;
    unsigned refcount;
    size_t totalBytes;
    int iovcnt;
    struct iovec array[];
};

struct ccnx_codec_network_buffer {
    size_t position;
    size_t capacity;         /**< Bytes allocated */

    CCNxCodecNetworkBufferMemory *current;
    CCNxCodecNetworkBufferMemory *head;
    CCNxCodecNetworkBufferMemory *tail;

    void *userarg;
    CCNxCodecNetworkBufferMemoryBlockFunctions memoryFunctions;
    unsigned refcount;
};


