SELECT
  cast(recvtimets as bigint) AS "time",
  cast(attrvalue as real) as attrvalue
FROM default_service.urn_ngsi_ld_oee_injectionmoulding1_oee
WHERE
  attrname = 'Quality' and
  recvtimets = (select recvtimets from default_service.urn_ngsi_ld_oee_injectionmoulding1_oee order by recvtimets desc limit 1)
ORDER BY 1
