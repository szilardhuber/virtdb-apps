
#include <logger.hh>
#include <connector.hh>
#include <chrono>
#include <thread>
#include <iostream>

using namespace virtdb::interface;
using namespace virtdb::util;
using namespace virtdb::connector;

namespace
{
  template <typename EXC>
  int usage(const EXC & exc)
  {
    std::cerr << "Exception: " << exc.what() << "\n"
              << "\n"
              << "Usage: testdata-service <ZeroMQ-EndPoint>\n"
              << "\n"
              << " endpoint examples: \n"
              << "  \"ipc:///tmp/diag-endpoint\"\n"
              << "  \"tcp://localhost:65001\"\n\n";
    return 100;
  }
  
  void add_field(pb::TableMeta & table,
                 const std::string & field,
                 pb::Kind kind)
  {
    auto f = table.add_fields();
    f->set_name(field);
    auto desc = f->mutable_desc();
    desc->set_type(kind);
  }
}

int main(int argc, char ** argv)
{
  try
  {
    if( argc < 2 )
    {
      THROW_("invalid number of arguments");
    }
    
    endpoint_client     ep_clnt(argv[1], "testdata-provider");
    log_record_client   log_clnt(ep_clnt, "diag-service");
    config_client       cfg_clnt(ep_clnt, "config-service");
    meta_data_server    meta_srv(cfg_clnt);
    query_server        query_srv(cfg_clnt);
    column_server       col_srv(cfg_clnt);
    
    meta_data_server::table_sptr table{new pb::TableMeta};
    table->set_name("test-table");
    table->set_schema("testdata");
    add_field(*table, "intfield", pb::Kind::INT32);
    add_field(*table, "strfield", pb::Kind::STRING);
    meta_srv.add_table(table);
    
    

    query_srv.watch("", [&](const std::string & provider_name,
                            query_server::query_sptr data) {
      std::string data_str{data->DebugString()};
      LOG_TRACE("Query arrived to" << V_(provider_name) << V_(data_str));
      // TODO : pass Query to column_server
      
      for( auto const & f : data->fields() )
      {
        std::shared_ptr<pb::Column> col{new pb::Column};
        col->set_queryid(data->queryid());
        col->set_name(f.name());
        auto desc = f.desc();
        auto val = col->mutable_data();
        switch( desc.type() )
        {
          case pb::Kind::INT32:
          {
            val->set_type(pb::Kind::INT32);
            for( int32_t i=0;i<100;++i )
              val->add_int32value(i);
            break;
          };

          default:
          case pb::Kind::STRING:
          {
            val->set_type(pb::Kind::STRING);
            for( int32_t i=0;i<100;++i )
              *(val->add_stringvalue()) = std::to_string(i);
            break;
          }
        }
        col->set_seqno(1);
        col->set_endofdata(true);
        std::ostringstream os;
        os << data->queryid() << " " << f.name();
        col_srv.publish(os.str(), col);
      }
    });
    
    while( true )
    {
      std::this_thread::sleep_for(std::chrono::seconds(60));
      LOG_TRACE("alive");
    }
  }
  catch (const std::exception & e)
  {
    return usage(e);
  }
  return 0;
}

