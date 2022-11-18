SELECT
  cast(recvtimets as bigint) AS "time",
  cast(attrvalue as real) as attrvalue
FROM default_service.urn_ngsi_ld_job_202200045_job
WHERE
  attrname = 'GoodPartCounter' and
  recvtimets = (select recvtimets from default_service.urn_ngsi_ld_job_202200045_job order by recvtimets desc limit 1)
ORDER BY 1
