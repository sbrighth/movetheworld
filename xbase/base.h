
#ifndef BASE_H_
#define BASE_H_

#ifdef __cplusplus
extern "C" {
#endif

const char* base_version();
int 		base_init( const char* arg );
void		base_set_flag( int flag );

void        msleep( uint64 time );

void        delayu( uint64 time );
void        delaym( uint64 time );
void        delays( uint64 time );

void        base_usleep( uint64 time );
void        base_msleep( uint64 time );
void        base_sleep( uint64 time );

void 		base_atof( const char* p, double* pdata );

int         base_start_timer( int index );
int         base_read_timer( int index, double *ptime );

const char* base_cmd_get_stdout( const char* pCmd );

int         base_sem_create( int key, int* pid );
int         base_sem_remove( int id );
int         base_sem_lock( int id );
int         base_sem_status( int id, int* pstat );
int         base_sem_is_lock( int id );
int         base_sem_is_unlock( int id );

int 		base_shm_create( int data_size, int key, int* pid );
int 		base_shm_remove( int id );
int 		base_shm_get( int id, int offset, int data_sz, char* data );
int 		base_shm_set( int id, int offset, int data_sz, char* data );

int			base_msgq_create( int key, int* pid );
int			base_msgq_remove( int id );
int			base_msgq_send( int id, void* msg, int msg_sz );
int			base_msgq_recv( int id, void* msg, int msg_sz, long msg_type, int msg_flag );

int 		base_mount( const char* host, const char* shdir, const char* path, const char* id, const char* passwd );
int 		base_unmount( const char* path );
int 		base_is_mount( const char* path );

int			_base_sock_sendrecv( const char* ip, int port, void* send_data, int send_size, void* recv_data, int recv_size, int timeout, int flag );
int			base_sock_send( const char* ip, int port, void* send_data, int send_size, int timeout );
int			base_sock_recv( const char* ip, int port, void* recv_data, int recv_size, int timeout );
int			base_sock_send_recv( const char* ip, int port, void* send_data, int send_size, void* recv_data, int recv_size, int timeout );
int 		base_sock_is_connect( const char* ip, int port, int timeout );
int			base_sock_get_ip( int eth_no, char *ip );

int         base_net_is_run( int eth_no );
int			base_net_set_ip( int eth_no, const char* ip );

int         base_file_is_exist( const char* p );
long        base_file_get_size( const char* p );
int			base_file_copy( const char* obj_file, const char* copy_file );
int			base_file_create( const char* obj_file );
int			base_file_remove( const char* obj_file );
int 		base_file_get_time( const char* obj_file, char* time_str );

long        base_file_write_bin( const char* p, char* info, long size );
long        base_file_update_bin( const char* p, char* info, long offset, long size );
long        base_file_read_bin( const char* p, char* info, long offset, long size );
long        base_file_add_bin( const char* p, char* info, long size );

long        base_file_write_text( const char* p, char* info, long size );
long        base_file_read_text( const char* p, char* info, long offset, long size );
long        base_file_add_text( const char* p, char* info, long size );

uint64 		base_rand_get_int( uint64 start_val, uint64 end_val );
double      base_rand_get_double( double start_val, double end_val );

int  		base_set_correct( double *dXx );
int			base_get_pattern( int index, void* buff, int len );
int			base_get_cpu_val( double* pdCpuPerVal );
int			base_get_proc_cnt( const char *pProcName );

int			base_str_is_contains( const char* obj_str, const char* find_str );

int 		base_timer_func( void* (*pfunc)(void*), int timeout );

const char*	base_path_get_ath_test( int id );
const char*	base_path_get_ath_in_file( int id );
const char*	base_path_get_ath_out_file( int id );

const char*	base_path_get_sha_test( int id );
const char*	base_path_get_sha_in_file( int id );
const char*	base_path_get_sha_out_file( int id );

int 		base_msg_init_header( const char* filename );
int 		base_msg_write( const char* filename, ExMsgPack* packet );

int			_base_msg_send( int port, ExMsgPack* msg_pack, long timeout, int flag );
int			base_msg_send( int port, ExMsgPack* msg_pack );
int			base_msg_send_recv( int port, ExMsgPack* msg_pack );
int			base_msg_display( const char* p );
const char*	base_msg_get_text( int msg );

int			_base_msg_test_status_send( int id, int port, int msg, int flag, const char* msg_str );
int			base_msg_test_start_send( int id, int port );
int			base_msg_test_stop_send( int id, int port );
int			base_msg_test_test_send( int id, int port, const char* msg_str );
int			base_msg_test_pass_send( int id, int port, const char* msg_str );
int			base_msg_test_fail_send( int id, int port, int flag, const char* msg_str );
int			base_msg_test_done_send( int id, int port, int flag, const char* msg_str );
int			base_msg_test_text_send( int id, int port, int index, const char* msg_str );

int			base_main_msg_send( int port, ExMsgPack* msg_pack );
int			base_debug_msg_send( int id, const char* cmd, char* p, int timeout );

int			base_pack_msgq_send( int id, ExMsgPack* packet );
int			base_pack_msgq_recv( int id, ExMsgPack* packet );
int			base_pack_msgq_remove( int id );

int			base_pack_d_msgq_send( int id, ExMsgPack* packet );
int			base_pack_d_msgq_recv( int id, ExMsgPack* packet );
int			base_pack_d_msgq_remove( int id );

int			base_set_correct_value( double *dXx );

#ifdef __cplusplus
}

#endif

#endif /* BASELIB_H_ */
