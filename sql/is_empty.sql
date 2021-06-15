select tname,tabtype from tab order by tname;
select sequence_name from user_sequences order by sequence_name;
select table_name||'.'||column_name from user_sdo_geom_metadata;