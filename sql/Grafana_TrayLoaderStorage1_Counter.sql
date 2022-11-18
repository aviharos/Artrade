SELECT
  cast(recvtimets as bigint) AS "time",
  cast(attrvalue as real) as attrvalue
FROM default_service.urn_ngsi_ld_storage_trayloaderstorage1_storage
WHERE
  attrname = 'Counter' and
  recvtimets = (select recvtimets from default_service.urn_ngsi_ld_storage_trayloaderstorage1_storage order by recvtimets desc limit 1)
ORDER BY 1
