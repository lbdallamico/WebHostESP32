# WiFi

Implementação de Wifi para operar com redes de internet

### Owner: Lucas Dallamico

Todas as configurações associadas a rede estão declaradas no `WiFi_Settings.h`

## Exemplo de uso com AP

```
WiFi * m_wifi = WiFi::GetObjs();
m_wifi->Begin(WiFi::kAP);
m_wifi->Start();
```

## Exemplo de uso com STA

```
WiFi * m_wifi = WiFi::GetObjs();
m_wifi->SetSsidAndPassword("RSSI_ROTEADOR", "SENHA");
m_wifi->Begin(WiFi::kSTA);
m_wifi->Start();
```

## Exemplo de uso do modo hibrido AP e STA

```
WiFi * m_wifi = WiFi::GetObjs();
m_wifi->SetSsidAndPassword("RSSI_ROTEADOR", "SENHA");
m_wifi->Begin(WiFi::kAP_STA);
m_wifi->Start();
```