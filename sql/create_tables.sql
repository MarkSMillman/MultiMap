spool create_tables.log

create table job (
id              number(10),
product         number(10),
name            varchar2(32),
target_folder   varchar2(1000);
description     varchar2(2048),
create_date     date,
launched_by     number(10),
status          number(2)
);
create table product (
id              number(10),
template        number(10),
scale           number(10),
file_name       varchar2(1000),
base_map        varchar2(128),
dpi             number(5),
global_width    number(8),
global_height   number(8),
origin_x        number(12,2),
origin_y        number(12,2),
name            varchar2(32),
edition         varchar2(32),
description     varchar2(2048)   
);
create table product_template (
id              number(10),
name            varchar2(32),
version         varchar2(32),
base_map        varchar2(128),
scale           number(10),
dpi             number(5),
global_width    number(8),
global_height   number(8),
description     varchar2(2048)   
);
create table action (
id              number(10),
product         number(10),
serial          number(10),
name            varchar2(32),
json            varchar2(4000)
);
spool off
