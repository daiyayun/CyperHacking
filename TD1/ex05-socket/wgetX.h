
#ifndef WGETX_H_
#define WGETX_H_

#define B_SIZE 1024 * 5000
#define USERAGENT "HTMLGET 1.0"
#define MAXRCVLEN 500

/**
 * \brief write the content to a file
 * \param path the path and name of the file
 * \param data the pointer of the buffer that to be written.
 */
void write_data(const char *path, const char *data);

/**
 * \brief download a page for a file through http protocol
 * \param info the url information
 * \param buff the buffe for keeping the downloaded file
 * \return the pointer to the downloaded file
 */
char* download_page(url_info info, char *buff);

char *get_ip(char *host);
char *build_get_query(char *host, char *path);

#endif /* WGETX_H_ */
