#ifndef	_NET_CAMERA_ERROR_H_
#define _NET_CAMERA_ERROR_H_

#define NC_ERROR_OK                             (000000)
#define NC_ERROR_UNKOWN_DEVICE                  (-20000)
#define NC_ERROR_INVALID_PARAM                  (-20001)
#define NC_ERROR_ALLOC_FAILED                   (-20002)
#define NC_ERROR_NULL_POINTER                   (-20003)
#define NC_ERROR_SOCKET                         (-20004)
#define NC_ERROR_DISCONNECT                     (-20005)
#define NC_ERROR_NO_IMAGE                       (-20006)
#define NC_ERROR_SEND_TIMEOUT                   (-20007)

#define NCServer_ERROR_OK                       (000000)
#define NCServer_ERROR_INVALID_PORT             (-30000)
#define NCServer_ERROR_QUEUE_FULL               (-30001)
#define NCServer_ERROR_QUEUE_EMPTY              (-30002)

#define NCStream_ERROR_OK                       (000000)
#define NCStream_ERROR_INVALID_URL              (-40000)
#define NCStream_ERROR_NULL_POINTER             (-40001)
#define NCStream_ERROR_INVALID_IP               (-40002)
#define NCStream_ERROR_INVALID_PORT             (-40003)
#define NCStream_ERROR_CREATE_CLIENT            (-40004)
#define NCStream_ERROR_SOCKET_COMM              (-40005)
#define NCStream_ERROR_GET_FRAME                (-40006)
#define NCStream_ERROR_NO_FRAME                 (-40007)

#define NCCodec_ERROR_OK                        (000000)
#define NCCodec_ERROR_CANNOT_FIND_DECODER       (-50000)
#define NCCodec_ERROR_CANNOT_INIT_DECODER       (-50001)
#define NCCodec_ERROR_DECODE_SPS_FAILED         (-50002)
#define NCCodec_ERROR_INVALID_SPS               (-50003)
#define NCCodec_ERROR_CANNOT_FIND_SPS           (-50004)
#define NCCodec_ERROR_GET_PIC_FAILED            (-50005)

#define NCCodec_ERROR_INVALID_PARAM             (-50100)


#define NCVideo_ERROR_OK                        (000000)
#define NCVideo_ERROR_INVALID_PARAM             (-60000)
#define	NCVideo_ERROR_OPEN_FILE_FAILED          (-60001)
#define NCVideo_ERROR_UNKOWN_FORMAT             (-60002)

#endif
