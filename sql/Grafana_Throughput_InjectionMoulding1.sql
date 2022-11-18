SELECT
  cast(recvtimets as bigint) AS "time",
  cast(attrvalue as real) as attrvalue
FROM default_service.urn_ngsi_ld_throughput_injectionmoulding1_throughput
WHERE
  attrname = 'Throughput' and
  recvtimets = (select recvtimets from default_service.urn_ngsi_ld_throughput_injectionmoulding1_throughput order by recvtimets desc limit 1)
ORDER BY 1
