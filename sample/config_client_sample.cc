
#include <logger.hh>
#include <util.hh>
#include <connector.hh>
#include <chrono>
#include <thread>
#include <iostream>

using namespace virtdb::util;
using namespace virtdb::connector;
using namespace virtdb::interface;

namespace
{
  template <typename EXC>
  int usage(const EXC & exc)
  {
    std::cerr << "Exception: " << exc.what() << "\n"
              << "\n"
              << "Usage: config_client_sample <ZeroMQ-EndPoint>\n"
              << "\n"
              << " endpoint examples: \n"
              << "  \"ipc:///tmp/cfg-endpoint\"\n"
              << "  \"tcp://localhost:65001\"\n\n";
    return 100;
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
    
    endpoint_client     ep_clnt(argv[1],  "config-client");
    log_record_client   log_clnt(ep_clnt, "diag-service");
    config_client       cfg_clnt(ep_clnt, "config-service");

    log_clnt.wait_valid_push();
    LOG_INFO("log connected");

    cfg_clnt.wait_valid_sub();
    cfg_clnt.wait_valid_req();
    LOG_INFO("config client connected");

    cfg_clnt.watch("*",
                   [](const std::string & provider_name,
                      const std::string & channel,
                      const std::string & subscription,
                      std::shared_ptr<pb::Config> cfg)
                   {
                     LOG_INFO("Received config (SUB):" << V_(provider_name) 
                               << V_(channel) 
                               << V_(subscription) 
                               << M_(*cfg));
                   });

    
    pb::Config cfg_req;
    cfg_req.set_name(ep_clnt.name());
    
    cfg_clnt.send_request(cfg_req, [](const pb::Config & cfg)
    {
      //std::cout << "Received config:\n" << cfg.DebugString() << "\n";
      return true;
    },1000);

    auto cfgdata = cfg_req.mutable_configdata();
    cfgdata->set_key("Hello");

    
    for( int i=0;i<4;++i )
    {
      // give a chance to log sender to initialize
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      auto mval = cfgdata->mutable_value();
      value_type<int32_t>::set(*mval, i);
      
      cfg_clnt.send_request(cfg_req,
                          [](const pb::Config & cfg)
      {
        // don't care. want to see SUB messages ...
        return true;
      },1000);
    }

    while(true)
    {
      std::this_thread::sleep_for(std::chrono::seconds(60));
      LOG_INFO("alive");
    }
    
    LOG_TRACE("exiting");
    ep_clnt.remove_watches();
    cfg_clnt.remove_watches();
  }
  catch (const std::exception & e)
  {
    return usage(e);
  }
  return 0;
}
