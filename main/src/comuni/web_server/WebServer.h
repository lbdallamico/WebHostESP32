#ifndef WebServer_H_
#define WebServer_H_

// #include "Glo"
#include <string>
#include "esp_log.h"
#include "esp_http_server.h"

class WebServer
{
public:
  bool Begin();
  bool Destruct();
  bool IsActive();
  bool GetRSSIfromHTMLpost(std::string rssi);
  bool GetPasswordfromHTMLpost(std::string password);
  static WebServer *GetObjs();

private:
  WebServer();
  /* http site */
  static esp_err_t handler_forms_http(httpd_req_t *req);
  static esp_err_t handler_submit_http(httpd_req_t *req);
  /* espected variables to receveid as parameter */
  std::string _rssi_from_html;
  std::string _password_from_html;
  bool _is_active;
  static const httpd_uri_t _uri_get;
  static const httpd_uri_t _uri_submit;
  httpd_handle_t handler_server; 
  /* Signleton */
  static WebServer * _ptr_web_server;
};

#endif