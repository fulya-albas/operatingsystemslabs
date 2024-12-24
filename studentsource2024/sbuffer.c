/**
 * \author {AUTHOR}
 */

#include <stdlib.h>
#include <stdio.h>
#include "sbuffer.h"
#include "config.h"

#include <inttypes.h>
#include <pthread.h>


/**
 * basic node for the buffer, these nodes are linked together to create the buffer
 */
typedef struct sbuffer_node {
	struct sbuffer_node *next;  /**< a pointer to the next node*/
	sensor_data_t data;         /**< a structure containing the data */
} sbuffer_node_t;

/**
 * a structure to keep track of the buffer
 */
struct sbuffer {
	sbuffer_node_t *head;       /**< a pointer to the first node in the buffer */
	sbuffer_node_t *tail;       /**< a pointer to the last node in the buffer */

	pthread_mutex_t mutex;
	pthread_cond_t not_empty;
	pthread_cond_t not_full;
	int is_open;

};

int sbuffer_init(sbuffer_t **buffer) {
	*buffer = malloc(sizeof(sbuffer_t));
	if (*buffer == NULL){
		return SBUFFER_FAILURE;
	}
	(*buffer)->head = NULL;
	(*buffer)->tail = NULL;

	pthread_mutex_init( &(*buffer)->mutex, NULL );
	pthread_cond_init( &(*buffer)->not_empty, NULL);
	pthread_cond_init(&(*buffer)->not_full, NULL);
	(*buffer)->is_open = 1 ;
	return SBUFFER_SUCCESS;
}

int buffer_is_open(sbuffer_t *buffer ){
	pthread_mutex_lock(&buffer->mutex);
	int is_open = buffer->is_open;
	pthread_mutex_unlock(&buffer->mutex );
	return is_open;
}


int sbuffer_free(sbuffer_t **buffer) {

	if((buffer == NULL) || (*buffer == NULL)) {
		return SBUFFER_FAILURE;
	}
	pthread_mutex_lock(&(*buffer)->mutex);
	(*buffer)->is_open = 0 ;
	pthread_cond_broadcast(&(*buffer)->not_empty);
	pthread_mutex_unlock(&(*buffer)->mutex);
	sbuffer_node_t *dummy ;
	while((*buffer )->head){
		dummy = (*buffer)->head ;
		(*buffer)->head = (*buffer )->head->next ;
		free(dummy );
	}
	pthread_mutex_destroy(&(*buffer)->mutex);
	pthread_cond_destroy(&(*buffer)->not_empty );
	pthread_cond_destroy(&(*buffer)-> not_full);
	free(*buffer) ;
	*buffer = NULL;
	return SBUFFER_SUCCESS;



}

int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data) {
	if(buffer == NULL ){
		return SBUFFER_FAILURE;
	}
	pthread_mutex_lock(&buffer->mutex);


	if(!buffer->is_open && buffer->head == NULL){
		pthread_mutex_unlock(&buffer->mutex );
		return SBUFFER_NO_DATA;
	}
	printf("trying 2 remove data\n");
	while(buffer->head == NULL && buffer->is_open ){
		pthread_cond_wait(&buffer->not_empty, &buffer->mutex );
	}
	if(buffer->head == NULL){
		pthread_mutex_unlock(&buffer->mutex);
		return SBUFFER_NO_DATA;
	}
	sbuffer_node_t *dummy = buffer->head;
	*data = dummy->data ;

	printf("removing data: id=%" PRIu16 ", value=%.2f, timestamp=%ld \n", data->id, data->value, (long)data->ts);
	if(buffer->head == buffer->tail){
		buffer->head = buffer->tail = NULL;
	}
	else{
		buffer->head = buffer->head->next ;
	}

	free(dummy);



	pthread_mutex_unlock(&buffer->mutex);

	printf("buffer after removal: head=%p, tail=%p \n", buffer->head, buffer->tail) ;

	return SBUFFER_SUCCESS;




}

int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data) {
	if(buffer == NULL ){
		return SBUFFER_FAILURE;
	}
	pthread_mutex_lock( &buffer->mutex);
	if(!buffer->is_open){
		pthread_mutex_unlock(&buffer->mutex);
		printf("buffer is closed, cldn't insert data \n");
		return SBUFFER_FAILURE;
	}
	sbuffer_node_t *dummy = malloc(sizeof( sbuffer_node_t));
	if(dummy == NULL){
		pthread_mutex_unlock(&buffer->mutex);
		printf("failed 2 allocate mem 4 new node \n");
		return SBUFFER_FAILURE;
	}
	dummy->data = *data ;
	dummy->next = NULL ;
	if(buffer->tail == NULL ){
		buffer->head = buffer->tail = dummy;
	}
	else{
		buffer->tail->next = dummy;
		buffer->tail = buffer->tail->next;
	}
	pthread_cond_signal(&buffer->not_empty );
	pthread_mutex_unlock(&buffer->mutex);
	return SBUFFER_SUCCESS;


}

int sbuffer_close(sbuffer_t *buffer){
	if(buffer == NULL ){
		return SBUFFER_FAILURE;
	}
	pthread_mutex_lock(&buffer->mutex);
	buffer->is_open = 0;
	pthread_cond_broadcast(&buffer->not_empty);
	pthread_mutex_unlock(&buffer->mutex);
	return SBUFFER_SUCCESS;
}
