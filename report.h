#ifndef _report_h__
#define _report_h__

typedef struct {

	int reportDescriptorSize;
	void *reportDescriptor; // must be in flash
	/**
	 * \param id controller id (starting at 1 to match report IDs)
	 * return The number of bytes written to buf
	 */
	char (*buildReport)(unsigned char *buf, char id);
} Report;

#endif // _report_h__


