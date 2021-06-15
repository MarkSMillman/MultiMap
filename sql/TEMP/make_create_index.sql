select 'create unique index '||c.index_name||' on '||c.table_name||'('||c.column_name||');' 
from user_indexes i, user_ind_columns c
where i.index_name=c.index_name and i.index_type not like 'DOMAIN' and i.uniqueness like 'UNIQUE';
select 'create index '||c.index_name||' on '||c.table_name||'('||c.column_name||');' 
from user_indexes i, user_ind_columns c
where i.index_name=c.index_name and i.index_type not like 'DOMAIN' and i.uniqueness like 'NONUNIQUE';
select 'create index '||c.index_name||' on '||c.table_name||'('||c.column_name||') indextype is mdsys.spatial_index;' 
from user_indexes i, user_ind_columns c
where i.index_name=c.index_name and i.index_type like 'DOMAIN';