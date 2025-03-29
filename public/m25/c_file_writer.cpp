#include "stdafx.h"
#include "c_file_writer.h"

#if 0

#include "../microlib/fs.h"

#endif

static bool __open(
	const char* in_filename, const char* in_mode,
	c_file_writer_t& out)
{
	//close any open handle
	out.close();

	/*
#ifdef _DEBUG
	++c_actual_disk_writes;
#endif
*/
	if (0 == ::fopen_s(&out._handle, in_filename, in_mode))
	{
		assert(out._handle);
		return true;
	}

	return false;
}

#if 0

//public
//public
//public
//public

#endif

c_file_writer_t::c_file_writer_t()
{
	_handle = nullptr;
}

c_file_writer_t::~c_file_writer_t()
{
	close();
}

#if 0

bool c_remove_file(const char* in_filename)
{
	if (0 == ::remove(in_filename))
	{
		//C_LOG_DEBUG("removed %s", in_filename);
		return true;
	}

	/*
	C_ERROR_DEBUG("failed to remove %s", in_filename);
	c_log_state.indent();

	int32_t e;
	if (0 == ::_get_errno(&e))
	{
		switch (e)
		{
		case ECHILD:
			C_ERROR_DEBUG("ECHILD: No spawned processes.");
			break;

		case EAGAIN:
			C_ERROR_DEBUG("EAGAIN: No more processes. An attempt to create a new process failed because there are no more process slots, or there is not enough memory, or the maximum nesting level has been reached.");
			break;

		case E2BIG:
			C_ERROR_DEBUG("E2BIG: Argument list too long.");
			break;

		case EACCES:
			C_ERROR_DEBUG("EACCES: Permission denied. The file's permission setting does not allow the specified access. This error signifies that an attempt was made to access a file (or, in some cases, a directory) in a way that is incompatible with the file's attributes.\nFor example, the error can occur when an attempt is made to read from a file that is not open, to open an existing read-only file for writing, or to open a directory instead of a file. Under MS-DOS operating system versions 3.0 and later, EACCES may also indicate a locking or sharing violation.\nThe error can also occur in an attempt to rename a file or directory or to remove an existing directory.");
			break;

		case EBADF:
			C_ERROR_DEBUG("EBADF: Bad file number. There are two possible causes: 1) The specified file descriptor is not a valid value or does not refer to an open file. 2) An attempt was made to write to a file or device opened for read-only access.");
			break;

		case EDEADLOCK:
			C_ERROR_DEBUG("EDEADLOCK: Resource deadlock would occur. The argument to a math function is not in the domain of the function.");
			break;

		case EDOM:
			C_ERROR_DEBUG("EDOM: Math argument.");
			break;

		case EEXIST:
			C_ERROR_DEBUG("EEXIST: Files exist. An attempt has been made to create a file that already exists. For example, the _O_CREAT and _O_EXCL flags are specified in an _open call, but the named file already exists.");
			break;

		case EILSEQ:
			C_ERROR_DEBUG("EILSEQ: Illegal sequence of bytes (for example, in an MBCS string).");
			break;

		case EINVAL:
			C_ERROR_DEBUG("EINVAL: Invalid argument. An invalid value was given for one of the arguments to a function. For example, the value given for the origin when positioning a file pointer (by means of a call to fseek) is before the beginning of the file.");
			break;

		case EMFILE:
			C_ERROR_DEBUG("EMFILE: Too many open files. No more file descriptors are available, so no more files can be opened.");
			break;

		case ENOENT:
			C_ERROR_DEBUG("ENOENT: No such file or directory. The specified file or directory does not exist or cannot be found. This message can occur whenever a specified file does not exist or a component of a path does not specify an existing directory.");
			return true;	//this is thus ok!
			break;

		case ENOEXEC:
			C_ERROR_DEBUG("ENOEXEC: Exec format error. An attempt was made to execute a file that is not executable or that has an invalid executable-file format.");
			break;

		case ENOMEM:
			C_ERROR_DEBUG("ENOMEM: Not enough core. Not enough memory is available for the attempted operator. For example, this message can occur when insufficient memory is available to execute a d3d9_descriptor_child process, or when the allocation request in a _getcwd call cannot be satisfied.");
			break;

		case ENOSPC:
			C_ERROR_DEBUG("ENOSPC: No space left on device. No more space for writing is available on the device (for example, when the disk is full).");
			break;

		case ERANGE:
			C_ERROR_DEBUG("ERANGE: Result too large. An argument to a math function is too large, resulting in partial or total loss of significance in the result. This error can also occur in other functions when an argument is larger than expected (for example, when the buffer argument to _getcwd is longer than expected).");
			break;

		case EXDEV:
			C_ERROR_DEBUG("EXDEV: Cross-device link. An attempt was made to move a file to a different device (using the rename function).");
			break;

		case STRUNCATE:
			C_ERROR_DEBUG("STRUNCATE: A string copy or concatenation resulted in a truncated string. See _TRUNCATE.");
			break;
		}
	}
	else
	{
		C_ERROR_DEBUG("_get_errno() failed");
	}

	c_log_state.undent();
	*/

	return false;
}

uint32_t c_string_disk_size(const char* in_string)
{
	if (in_string)
	{
		return
			::strlen(in_string) * sizeof(char) +
			sizeof(uint32_t);	//length
	}

	return sizeof(uint32_t);	//0 length
}

#endif

void c_file_writer_t::close()
{
	if (_handle)
	{
		::fclose(_handle);
		_handle = nullptr;
	}
}

#if 0

bool c_file_writer_t::open_binary(const char* in_filename)
{
	return __open(in_filename, "wb", *this);
}

bool c_file_writer_t::bytes(const void* in_bytes, const uint32_t in_size)
{
	return
		nullptr != _handle &&
		0 < in_size &&
		1 == ::fwrite(in_bytes, in_size, 1, _handle);
}

#endif

bool c_file_writer_t::open_text(const char* in_filename)
{
	if (__open(in_filename, "wt", *this))
	{
		assert(_handle);
		//if (aDisableBuffering)
		//	::setvbuf(file.handle, nullptr, _IONBF, 0);
		return true;
	}

	return false;
}

#if 0

bool c_file_writer_t::string(const char* in_string)
{
	if (in_string)
	{
		const uint32_t LENGTH = ::strlen(in_string);

		if (!C_FILE_WRITE(LENGTH, *this))
			return false;

		if (LENGTH)
		{
			if (!bytes(in_string, LENGTH * sizeof(char)))
				return false;
		}
	}
	else
	{
		const uint32_t ZERO_LENGTH = 0;

		if (!C_FILE_WRITE(ZERO_LENGTH, *this))
			return false;
	}

	return true;
}

bool c_file_writer_t::blob(const c_blob_t& in)
{
	return
		C_FILE_WRITE(in.size, *this) &&
		bytes(in.data, in.size);
}

bool c_bytes_to_file(const void* in_src_bytes, const uint32_t in_src_size, const char* in_file)
{
	assert(in_src_bytes);
	assert(in_src_size);
	bool result = false;
	c_file_writer_t out;
	if (out.open_binary(in_file))
	{
		result = out.bytes(in_src_bytes, in_src_size);
		out.close();
	}
	return result;
}

bool c_bytes_to_file(const c_blob_t& in_blob, const char* in_file)
{
	if (
		in_blob.data &&
		in_blob.size
		)
		return c_bytes_to_file(in_blob.data, in_blob.size, in_file);
	return c_remove_file(in_file);
}

bool c_bytes_to_file(const c_managed_blob_t& in_blob, const char* in_file)
{
	if (
		in_blob.data &&
		in_blob.size
		)
		return c_bytes_to_file(in_blob.data, in_blob.size, in_file);
	return c_remove_file(in_file);
}

#ifdef _DEBUG
uint32_t c_actual_disk_writes = 0;
#endif

#endif
