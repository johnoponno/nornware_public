#pragma once

struct c_blob_t;

#if 0

struct c_managed_blob_t;

#endif

struct c_file_writer_t
{
	explicit c_file_writer_t();
	~c_file_writer_t();

	//bool open_binary(const char* aFile);
	bool open_text(const char* aFile);
	//bool bytes(const void* aSrc, const uint32_t aSize);
	//bool blob(const c_blob_t& aSource);
	//bool string(const char* aString);
	void close();

	::FILE* _handle;
};

#if 0

bool c_remove_file(const char* aFileName);
uint32_t c_string_disk_size(const char* aString);

bool c_bytes_to_file(const void* in_src_bytes, const uint32_t in_src_size, const char* in_file);
bool c_bytes_to_file(const c_blob_t& in_blob, const char* in_file);
bool c_bytes_to_file(const c_managed_blob_t& in_blob, const char* in_file);

#ifdef _DEBUG
extern uint32_t c_actual_disk_writes;
#endif

#define C_FILE_WRITE(instance, file) (file).bytes(&instance, sizeof(instance))
#define C_BYTES_TO_FILE(instance, file) c_bytes_to_file(&instance, sizeof(instance), file)

#endif
