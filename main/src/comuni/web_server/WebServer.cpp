#include "WebServer.h"
/*
 * ============================================================
 *                      Static Variables
 * ============================================================
 */
#define TAG "[WebServer]"
WebServer *WebServer::_ptr_web_server = nullptr;

/* Aqui é o que vai aparecer no celular */
std::string pagina_html = R"(
<!DOCTYPE html>
<html>
<body>
  <form action='/submit' method='post'>
    <label for='input1'>Campo 1:</label>
    <input type='text' id='input1' name='campo1'><br><br>
    <label for='input2'>Campo 2:</label>
    <input type='text' id='input2' name='campo2'><br><br>
    <input type='submit' value='Enviar'>
  </form>
</body>
</html>
)";

/* URI handler structure for GET / */
const httpd_uri_t WebServer::_uri_get = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = handler_forms_http,
    .user_ctx = NULL
};

/* URI handler structure for GET /submit */
const httpd_uri_t WebServer::_uri_submit = {
    .uri = "/submit",
    .method = HTTP_GET,
    .handler = handler_submit_http,
    .user_ctx = NULL
};

/*
 * ============================================================
 *                    Public Implementations
 * ============================================================
 */

bool WebServer::Begin()
{
  // Init
  handler_server = NULL;
  _is_active = false;
  _rssi_from_html = "";
  _password_from_html = "";

  // Get default config
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.lru_purge_enable = true;
  config.max_open_sockets = 2;
  config.stack_size = 1024 * 24;

  ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
  if (httpd_start(&handler_server, &config) == ESP_OK)
  {
    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_register_uri_handler(handler_server, &_uri_get);
    _is_active = true;
    return true;
  }
  else
  {
    ESP_LOGI(TAG, "Error starting server!");
    return false;
  }
}

bool WebServer::Destruct()
{
  if (_is_active == true)
  {
    httpd_stop(handler_server);
    _is_active = false;
    return true;
  }
  else
  {
    return false;
  }
}

bool WebServer::GetRSSIfromHTMLpost(std::string &rssi)
{
  rssi = _rssi_from_html;
  return true;
}

bool WebServer::GetPasswordfromHTMLpost(std::string &password)
{
  password = _password_from_html;
  return true;
}

WebServer *WebServer::GetObjs()
{
  if (_ptr_web_server == nullptr)
  {
    _ptr_web_server = new WebServer;
    if (_ptr_web_server == nullptr)
    {
      ESP_LOGE(TAG, "Erro to alloc WebServer");
    }
  }
  return _ptr_web_server;
}

bool WebServer::IsActive()
{
  return _is_active;
}

/*
 * ============================================================
 *                    Private Implementations
 * ============================================================
 */
WebServer::WebServer()
{
}

esp_err_t WebServer::handler_forms_http(httpd_req_t *req)
{
  httpd_resp_set_type(req, "text/html");
  httpd_resp_send(req, pagina_html.c_str(), -1); // Envia a página HTML
  return ESP_OK;
}

esp_err_t WebServer::handler_submit_http(httpd_req_t *req)
{
  return ESP_OK;
}