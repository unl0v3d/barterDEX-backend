
/******************************************************************************
 * Copyright © 2014-2017 The SuperNET Developers.                             *
 *                                                                            *
 * See the AUTHORS, DEVELOPER-AGREEMENT and LICENSE files at                  *
 * the top-level directory of this distribution for the individual copyright  *
 * holder information and the developer policies on copyright and licensing.  *
 *                                                                            *
 * Unless otherwise agreed in a custom licensing agreement, no part of the    *
 * SuperNET software, including this file may be copied, modified, propagated *
 * or distributed except according to the terms contained in the LICENSE file *
 *                                                                            *
 * Removal or modification of this copyright notice is prohibited.            *
 *                                                                            *
 ******************************************************************************/
//
//  LP_commands.c
//  marketmaker
//

char *LP_numutxos()
{
    cJSON *retjson = cJSON_CreateObject();
    if ( LP_mypeer != 0 )
    {
        jaddstr(retjson,"ipaddr",LP_mypeer->ipaddr);
        jaddnum(retjson,"port",LP_mypeer->port);
        //jaddnum(retjson,"numutxos",LP_mypeer->numutxos);
        jaddnum(retjson,"numpeers",LP_mypeer->numpeers);
        jaddnum(retjson,"session",G.LP_sessionid);
    } else jaddstr(retjson,"error","client node");
    return(jprint(retjson,1));
}

char *stats_JSON(void *ctx,char *myipaddr,int32_t pubsock,cJSON *argjson,char *remoteaddr,uint16_t port) // from rpc port
{
    char *method,*userpass,*base,*rel,*coin,*passphrase,*retstr = 0; int32_t authenticated=0,changed,flag = 0; cJSON *retjson,*reqjson = 0; struct iguana_info *ptr;
    method = jstr(argjson,"method");
    if ( method != 0 && (strcmp(method,"addr_unspents") == 0 || strcmp(method,"uitem") == 0 || strcmp(method,"postutxos") == 0) )
        return(0);
//printf("stats_JSON.(%s)\n",jprint(argjson,0));
    /*if ( (ipaddr= jstr(argjson,"ipaddr")) != 0 && (argport= juint(argjson,"port")) != 0 && (method == 0 || strcmp(method,"electrum") != 0) )
    {
        if ( strcmp(ipaddr,"127.0.0.1") != 0 && argport >= 1000 )
        {
            flag = 1;
            if ( (pushport= juint(argjson,"push")) == 0 )
                pushport = argport + 1;
            if ( (subport= juint(argjson,"sub")) == 0 )
                subport = argport + 2;
            if ( (peer= LP_peerfind((uint32_t)calc_ipbits(ipaddr),argport)) != 0 )
            {
                if ( 0 && (otherpeers= jint(argjson,"numpeers")) > peer->numpeers )
                    peer->numpeers = otherpeers;
                if ( peer->sessionid == 0 )
                    peer->sessionid = juint(argjson,"session");
                //printf("peer.(%s) found (%d %d) (%d %d) (%s)\n",peer->ipaddr,peer->numpeers,peer->numutxos,otherpeers,othernumutxos,jprint(argjson,0));
            } else LP_addpeer(LP_mypeer,LP_mypubsock,ipaddr,argport,pushport,subport,jint(argjson,"numpeers"),jint(argjson,"numutxos"),juint(argjson,"session"));
        }
    }*/
    if ( method == 0 )
    {
        if ( is_cJSON_Array(argjson) != 0 )
            printf("RAWARRAY command? %s\n",jprint(argjson,0));
        if ( flag == 0 || jobj(argjson,"result") != 0 )
            printf("stats_JSON no method: (%s)\n",jprint(argjson,0));
        return(0);
    }
    if ( strcmp(method,"hello") == 0 )
    {
        //int32_t i; cJSON *array = cJSON_CreateArray();
        retjson = cJSON_CreateObject();
        jaddstr(retjson,"result","success");
        jaddstr(retjson,"status","got hello");
        //for (i=0; i<10000; i++)
        //    jaddinum(array,i);
        //jadd(retjson,"array",array);
        return(jprint(retjson,1));
        //printf("got hello from %s:%u\n",ipaddr!=0?ipaddr:"",argport);
        //return(clonestr("{\"result\":\"success\",\"status\":\"got hello\"}"));
    }
    /*else if ( strcmp(method,"sendmessage") == 0 && jobj(argjson,"userpass") == 0 )
     {
         static char *laststr;
         char *newstr; bits256 pubkey = jbits256(argjson,"pubkey");
         if ( bits256_nonz(pubkey) == 0 || bits256_cmp(pubkey,G.LP_mypub25519) == 0 )
         {
             newstr = jprint(argjson,0);
             if ( laststr == 0 || strcmp(laststr,newstr) != 0 )
             {
                 printf("got message.(%s) from %s:%u\n",newstr,ipaddr!=0?ipaddr:"",argport);
                 if ( laststr != 0 )
                     free(laststr);
                 laststr = newstr;
                 LP_gotmessage(argjson);
                 retstr = clonestr(laststr);
             }
         } else retstr = clonestr("{\"error\":\"duplicate message\"}");
     }*/
    //else if ( strcmp(method,"nn_tests") == 0 )
    //    return(clonestr("{\"result\":\"success\"}"));
     else if ( strcmp(method,"help") == 0 )
         return(clonestr("{\"result\":\" \
available localhost RPC commands: \n \
setprice(base, rel, price, broadcast=1)\n\
autoprice(base, rel, fixed, minprice, maxprice, margin, refbase, refrel, factor, offset)*\n\
goal(coin=*, val=<autocalc>)\n\
myprice(base, rel)\n\
enable(coin)\n\
disable(coin)\n\
notarizations(coin)\n\
statsdisp(starttime=0, endtime=0, gui="", pubkey="", base="", rel="")\n\
ticker(base="", rel="")\n\
tradesarray(base, rel, starttime=<now>-timescale*1024, endtime=<now>, timescale=60) -> [timestamp, high, low, open, close, relvolume, basevolume, aveprice, numtrades]\n\
pricearray(base, rel, starttime=0, endtime=0, timescale=60) -> [timestamp, avebid, aveask, highbid, lowask]\n\
getrawtransaction(coin, txid)\n\
inventory(coin, reset=0, [passphrase=])\n\
lastnonce()\n\
buy(base, rel, price, relvolume, timeout=10, duration=3600, nonce)\n\
sell(base, rel, price, basevolume, timeout=10, duration=3600, nonce)\n\
withdraw(coin, outputs[])\n\
sendrawtransaction(coin, signedtx)\n\
swapstatus(pending=0)\n\
swapstatus(coin, limit=10)\n\
swapstatus(base, rel, limit=10)\n\
swapstatus(requestid, quoteid, pending=0)\n\
recentswaps(limit=3)\n\
notarizations(coin)\n\
public API:\n \
getcoins()\n\
getcoin(coin)\n\
portfolio()\n\
getpeers()\n\
passphrase(passphrase, gui, netid=0, seednode="")\n\
listunspent(coin, address)\n\
setconfirms(coin, numconfirms, maxconfirms=6)\n\
trust(pubkey, trust) # positive to trust, 0 for normal, negative to blacklist\n\
balance(coin, address)\n\
balances(address)\n\
fundvalue(address="", holdings=[], divisor=0)\n\
orderbook(base, rel, duration=3600)\n\
getprices()\n\
getmyprice(base, rel)\n\
getprice(base, rel)\n\
//sendmessage(base=coin, rel="", pubkey=zero, <argjson method2>)\n\
//getmessages(firsti=0, num=100)\n\
//deletemessages(firsti=0, num=100)\n\
secretaddresses(prefix='secretaddress', passphrase, num=10, pubtype=60, taddr=0)\n\
electrum(coin, ipaddr, port)\n\
snapshot(coin, height)\n\
snapshot_balance(coin, height, addresses[])\n\
dividends(coin, height, <args>)\n\
stop()\n\
bot_list()\n\
bot_statuslist()\n\
bot_buy(base, rel, maxprice, relvolume) -> botid\n\
bot_sell(base, rel, minprice, basevolume) -> botid\n\
bot_settings(botid, newprice, newvolume)\n\
bot_status(botid)\n\
bot_stop(botid)\n\
bot_pause(botid)\n\
instantdex_deposit(weeks, amount, broadcast=1)\n\
instantdex_claim()\n\
jpg(srcfile, destfile, power2=7, password, data="", required, ind=0)\n\
\"}"));
    //sell(base, rel, price, basevolume, timeout=10, duration=3600)\n\
    
    if ( (base= jstr(argjson,"base")) == 0 )
        base = "";
    if ((rel= jstr(argjson,"rel")) == 0 )
        rel = "";
    if ( (coin= jstr(argjson,"coin")) == 0 )
        coin = "";
    if ( G.USERPASS[0] != 0 && strcmp(remoteaddr,"127.0.0.1") == 0 && port != 0 && strcmp(method,"psock") != 0 ) // protected localhost
    {
        if ( G.USERPASS_COUNTER == 0 )
        {
            char pub33str[67];
            G.USERPASS_COUNTER = 1;
            retjson = cJSON_CreateObject();
            jaddstr(retjson,"userpass",G.USERPASS);
            jaddbits256(retjson,"mypubkey",G.LP_mypub25519);
            init_hexbytes_noT(pub33str,G.LP_pubsecp,33);
            jaddstr(retjson,"pubsecp",pub33str);
            jadd(retjson,"coins",LP_coinsjson(LP_showwif));
            LP_cmdcount++;
            return(jprint(retjson,1));
        }
        // if passphrase api and passphrase is right, ignore userpass, use hass of passphrase
        if ( strcmp(method,"passphrase") == 0 && (passphrase= jstr(argjson,"passphrase")) != 0 )
        {
            bits256 passhash; char str[65],str2[65];
            vcalc_sha256(0,passhash.bytes,(uint8_t *)passphrase,(int32_t)strlen(passphrase));
            if ( bits256_cmp(passhash,G.LP_passhash) == 0 )
                authenticated = 1;
            else printf("passhash %s != G %s\n",bits256_str(str,passhash),bits256_str(str2,G.LP_passhash));
        }
        if ( authenticated == 0 && ((userpass= jstr(argjson,"userpass")) == 0 || strcmp(userpass,G.USERPASS) != 0) )
            return(clonestr("{\"error\":\"authentication error you need to make sure userpass is set\"}"));
        if ( jobj(argjson,"userpass") != 0 )
            jdelete(argjson,"userpass");
        LP_cmdcount++;
        if ( strcmp(method,"passphrase") == 0 )
        {
            char coinaddr[64],pub33str[67];
            G.USERPASS_COUNTER = 1;
            if ( LP_passphrase_init(jstr(argjson,"passphrase"),jstr(argjson,"gui"),juint(argjson,"netid"),jstr(argjson,"seednode")) < 0 )
                return(clonestr("{\"error\":\"couldnt change passphrase\"}"));
            {
                retjson = cJSON_CreateObject();
                jaddstr(retjson,"result","success");
                jaddstr(retjson,"userpass",G.USERPASS);
                jaddbits256(retjson,"mypubkey",G.LP_mypub25519);
                init_hexbytes_noT(pub33str,G.LP_pubsecp,33);
                jaddstr(retjson,"pubsecp",pub33str);
                bitcoin_address("KMD",coinaddr,0,60,G.LP_myrmd160,20);
                jaddstr(retjson,"KMD",coinaddr);
                bitcoin_address("BTC",coinaddr,0,0,G.LP_myrmd160,20);
                jaddstr(retjson,"BTC",coinaddr);
                jaddstr(retjson,"NXT",G.LP_NXTaddr);
                jadd(retjson,"coins",LP_coinsjson(LP_showwif));
                return(jprint(retjson,1));
            }
        }
        else if ( strcmp(method,"instantdex_deposit") == 0 )
        {
            if ( (ptr= LP_coinsearch("KMD")) != 0 )
            {
                if ( jint(argjson,"weeks") <= 0 || jdouble(argjson,"amount") < 10. )
                    return(clonestr("{\"error\":\"instantdex_deposit needs to have weeks and amount\"}"));
                else return(LP_instantdex_deposit(ptr,juint(argjson,"weeks"),jdouble(argjson,"amount"),jobj(argjson,"broadcast") != 0 ? jint(argjson,"broadcast") : 1));
            }
            return(clonestr("{\"error\":\"cant find KMD\"}"));
        }
        else if ( strcmp(method,"instantdex_claim") == 0 )
        {
            if ( (ptr= LP_coinsearch("KMD")) != 0 )
            {
                return(LP_instantdex_claim(ptr));
            }
            return(clonestr("{\"error\":\"cant find KMD\"}"));
        }
        else if ( strcmp(method,"jpg") == 0 )
        {
            return(LP_jpg(jstr(argjson,"srcfile"),jstr(argjson,"destfile"),jint(argjson,"power2"),jstr(argjson,"password"),jstr(argjson,"data"),jint(argjson,"required"),juint(argjson,"ind")));
        }
        /*else if ( strcmp(method,"sendmessage") == 0 )
        {
            if ( jobj(argjson,"method2") == 0 )
            {
                LP_broadcast_message(LP_mypubsock,base!=0?base:coin,rel,jbits256(argjson,"pubkey"),jprint(argjson,0));
            }
            return(clonestr("{\"result\":\"success\"}"));
        }
        else if ( strcmp(method,"getmessages") == 0 )
        {
            if ( (retjson= LP_getmessages(jint(argjson,"firsti"),jint(argjson,"num"))) != 0 )
                return(jprint(retjson,1));
            else return(clonestr("{\"error\":\"null messages\"}"));
        }
        else if ( strcmp(method,"deletemessages") == 0 )
        {
            LP_deletemessages(jint(argjson,"firsti"),jint(argjson,"num"));
            return(clonestr("{\"result\":\"success\"}"));
        }*/
        else if ( strcmp(method,"recentswaps") == 0 )
        {
            return(LP_recent_swaps(jint(argjson,"limit")));
        }
        else if ( strcmp(method,"stop") == 0 )
        {
            printf("DEBUG stop\n");
            LP_STOP_RECEIVED = 1;
            return(clonestr("{\"result\":\"success\"}"));
        }
        else if ( strcmp(method,"millis") == 0 )
        {
            LP_millistats_update(0);
            return(clonestr("{\"result\":\"success\"}"));
        }
        else if ( strcmp(method,"getprices") == 0 )
            return(LP_prices());
        else if ( strcmp(method,"getpeers") == 0 )
            return(LP_peers());
        else if ( strcmp(method,"getcoins") == 0 )
            return(jprint(LP_coinsjson(0),1));
        else if ( strcmp(method,"notarizations") == 0 )
        {
            if ( (ptr= LP_coinsearch(coin)) != 0 )
            {
                retjson = cJSON_CreateObject();
                jaddstr(retjson,"result","success");
                jaddstr(retjson,"coin",coin);
                jaddnum(retjson,"lastnotarization",ptr->notarized);
                jaddnum(retjson,"bestheight",ptr->height);
                return(jprint(retjson,1));
            } else return(clonestr("{\"error\":\"cant find coin\"}"));
        }
        else if ( strcmp(method,"portfolio") == 0 )
        {
            return(LP_portfolio());
        }
        else if ( strcmp(method,"statsdisp") == 0 )
        {
            return(jprint(LP_statslog_disp(juint(argjson,"starttime"),juint(argjson,"endtime"),jstr(argjson,"gui"),jbits256(argjson,"pubkey"),jstr(argjson,"base"),jstr(argjson,"rel")),1));
        }
        else if ( strcmp(method,"ticker") == 0 )
        {
            return(LP_ticker(jstr(argjson,"base"),jstr(argjson,"rel")));
        }
        else if ( strcmp(method,"secretaddresses") == 0 )
        {
            uint8_t taddr,pubtype;
            pubtype = (jobj(argjson,"pubtype") == 0) ? 60 : juint(argjson,"pubtype");
            taddr = (jobj(argjson,"taddr") == 0) ? 0 : juint(argjson,"taddr");
            return(LP_secretaddresses(ctx,jstr(argjson,"prefix"),jstr(argjson,"passphrase"),juint(argjson,"num"),taddr,pubtype));
        }
        else if ( strcmp(method,"swapstatus") == 0 )
        {
            uint32_t requestid,quoteid;
            if ( (requestid= juint(argjson,"requestid")) != 0 && (quoteid= juint(argjson,"quoteid")) != 0 )
                return(basilisk_swapentry(requestid,quoteid,1));
            else if ( coin[0] != 0 )
                return(basilisk_swapentries(coin,0,jint(argjson,"limit")));
            else if ( base[0] != 0 && rel[0] != 0 )
                return(basilisk_swapentries(base,rel,jint(argjson,"limit")));
            else return(basilisk_swaplist(0,0,0,jint(argjson,"pending")));
        }
        else if ( strcmp(method,"dynamictrust") == 0 )
        {
            struct LP_address *ap; char *coinaddr;
            if ( (ptr= LP_coinsearch("KMD")) != 0 && (coinaddr= jstr(argjson,"address")) != 0 )
            {
                //LP_zeroconf_deposits(ptr);
                if ( (ap= LP_addressfind(ptr,coinaddr)) != 0 )
                {
                    retjson = cJSON_CreateObject();
                    jaddstr(retjson,"result","success");
                    jaddstr(retjson,"address",coinaddr);
                    jaddnum(retjson,"zcredits",dstr(ap->instantdex_credits));
                    return(jprint(retjson,1));
                }
            }
            return(clonestr("{\"error\":\"cant find address\"}"));
        }
        else if ( (retstr= LP_istradebots_command(ctx,pubsock,method,argjson)) != 0 )
            return(retstr);
        if ( base[0] != 0 && rel[0] != 0 )
        {
            double price,bid,ask;
            if ( strcmp(method,"autoprice") == 0 )
            {
                if ( LP_autoprice(ctx,base,rel,argjson) < 0 )
                    return(clonestr("{\"error\":\"couldnt set autoprice\"}"));
                else return(clonestr("{\"result\":\"success\"}"));
            }
            else if ( strcmp(method,"pricearray") == 0 )
            {
                uint32_t firsttime;
                if ( base[0] != 0 && rel[0] != 0 )
                {
                    if ( (firsttime= juint(argjson,"starttime")) < time(NULL)-30*24*3600 )
                        firsttime = (uint32_t)(time(NULL)-30*24*3600);
                    return(jprint(LP_pricearray(base,rel,firsttime,juint(argjson,"endtime"),jint(argjson,"timescale")),1));
                } else return(clonestr("{\"error\":\"pricearray needs base and rel\"}"));
            }
            else if ( strcmp(method,"tradesarray") == 0 )
            {
                return(jprint(LP_tradesarray(base,rel,juint(argjson,"starttime"),juint(argjson,"endtime"),jint(argjson,"timescale")),1));
            }
            if ( IAMLP == 0 && LP_isdisabled(base,rel) != 0 )
                return(clonestr("{\"error\":\"at least one of coins disabled\"}"));
            price = jdouble(argjson,"price");
            if ( strcmp(method,"setprice") == 0 )
            {
                if ( LP_mypriceset(&changed,base,rel,price) < 0 )
                    return(clonestr("{\"error\":\"couldnt set price\"}"));
                //else if ( LP_mypriceset(&changed,rel,base,1./price) < 0 )
                //    return(clonestr("{\"error\":\"couldnt set price\"}"));
                else if ( price == 0. || jobj(argjson,"broadcast") == 0 || jint(argjson,"broadcast") != 0 )
                    return(LP_pricepings(ctx,myipaddr,LP_mypubsock,base,rel,price * LP_profitratio));
                else return(clonestr("{\"result\":\"success\"}"));
            }
            else if ( strcmp(method,"orderbook") == 0 )
                return(LP_orderbook(base,rel,jint(argjson,"duration")));
            else if ( strcmp(method,"myprice") == 0 )
            {
                if ( LP_myprice(&bid,&ask,base,rel) > SMALLVAL )
                {
                    retjson = cJSON_CreateObject();
                    jaddstr(retjson,"base",base);
                    jaddstr(retjson,"rel",rel);
                    jaddnum(retjson,"bid",bid);
                    jaddnum(retjson,"ask",ask);
                    return(jprint(retjson,1));
                } else return(clonestr("{\"error\":\"no price set\"}"));
            }
            else if ( strcmp(method,"buy") == 0 )
            {
                //*
                if ( price > SMALLVAL )
                {
                    return(LP_autobuy(ctx,myipaddr,pubsock,base,rel,price,jdouble(argjson,"relvolume"),jint(argjson,"timeout"),jint(argjson,"duration"),jstr(argjson,"gui"),juint(argjson,"nonce"),jbits256(argjson,"destpubkey"),0));
                } else return(clonestr("{\"error\":\"no price set\"}"));
            }
            else if ( strcmp(method,"sell") == 0 )
            {
                //*
                if ( price > SMALLVAL )
                {
                    return(LP_autobuy(ctx,myipaddr,pubsock,rel,base,1./price,jdouble(argjson,"basevolume"),jint(argjson,"timeout"),jint(argjson,"duration"),jstr(argjson,"gui"),juint(argjson,"nonce"),jbits256(argjson,"destpubkey"),0));
                } else return(clonestr("{\"error\":\"no price set\"}"));
            }
        }
        /*else if ( rel[0] != 0 && strcmp(method,"bestfit") == 0 )
        {
            double relvolume;
            if ( (relvolume= jdouble(argjson,"relvolume")) > SMALLVAL )
                return(LP_bestfit(rel,relvolume));
            else return(clonestr("{\"error\":\"no relvolume set\"}"));
        }*/
        else if ( coin[0] != 0 )
        {
            if ( strcmp(method,"enable") == 0 )
            {
                //*
                if ( (ptr= LP_coinsearch(coin)) != 0 )
                {
                    if ( ptr->userpass[0] == 0 && strcmp(ptr->symbol,"ETH") != 0 )
                    {
                        cJSON *retjson = cJSON_CreateObject();
                        jaddstr(retjson,"error",LP_DONTCHANGE_ERRMSG0);
                        jaddstr(retjson,"coin",coin);
                        return(jprint(retjson,1));
                    }
                    if ( LP_conflicts_find(ptr) == 0 )
                    {
                        ptr->inactive = 0;
                        cJSON *array; int32_t notarized;
                        if ( strcmp(ptr->symbol,"ETH") != 0 && LP_getheight(&notarized,ptr) <= 0 )
                        {
                            ptr->inactive = (uint32_t)time(NULL);
                            return(clonestr("{\"error\":\"coin cant be activated till synced\"}"));
                        }
                        else
                        {
                            if ( ptr->smartaddr[0] != 0 )
                                LP_unspents_load(coin,ptr->smartaddr);
                            LP_unspents_load(coin,ptr->smartaddr);
                            if ( strcmp(ptr->symbol,"KMD") == 0 )
                                LP_importaddress("KMD",BOTS_BONDADDRESS);
                        }
                        array = cJSON_CreateArray();
                        jaddi(array,LP_coinjson(ptr,0));
                        return(jprint(array,1));
                    } else return(clonestr("{\"error\":\"coin port conflicts with existing coin\"}"));
                } else return(clonestr("{\"error\":\"couldnt find coin\"}"));
            }
            else if ( strcmp(method,"disable") == 0 )
            {
                //*
                if ( (ptr= LP_coinsearch(coin)) != 0 )
                {
                    ptr->inactive = (uint32_t)time(NULL);
                    cJSON *array = cJSON_CreateArray();
                    jaddi(array,LP_coinjson(ptr,0));
                    return(jprint(array,1));
                } else return(clonestr("{\"error\":\"couldnt find coin\"}"));
            }
            else if ( strcmp(method,"listunspent") == 0 )
            {
                if ( (ptr= LP_coinsearch(coin)) != 0 )
                {
                    char *coinaddr; bits256 zero;
                    memset(zero.bytes,0,sizeof(zero));
                    if ( (coinaddr= jstr(argjson,"address")) != 0 )
                    {
                        if ( coinaddr[0] != 0 )
                        {
                            LP_address(ptr,coinaddr);
                            if ( strcmp(coinaddr,ptr->smartaddr) == 0 && bits256_nonz(G.LP_privkey) != 0 )
                            {
                                LP_listunspent_issue(coin,coinaddr,2,zero,zero);
                                //LP_privkey_init(-1,ptr,G.LP_privkey,G.LP_mypub25519);
                            }
                            return(jprint(LP_listunspent(coin,coinaddr,zero,zero),1));
                        }
                    }
                    return(clonestr("{\"error\":\"no address specified\"}"));
                } else return(clonestr("{\"error\":\"cant find coind\"}"));
            }
            else if ( strcmp(method,"balance") == 0 )
            {
                if ( (ptr= LP_coinsearch(coin)) != 0 )
                    return(jprint(LP_address_balance(ptr,jstr(argjson,"address"),1),1));
                else return(clonestr("{\"error\":\"cant find coind\"}"));
            }
            else if ( strcmp(method,"electrum") == 0 )
            {
                if ( (ptr= LP_coinsearch(coin)) != 0 )
                {
                    ptr->inactive = 0;
                    return(jprint(LP_electrumserver(ptr,jstr(argjson,"ipaddr"),juint(argjson,"port")),1));
                } else return(clonestr("{\"error\":\"cant find coind\"}"));
            }
            else if ( strcmp(method,"sendrawtransaction") == 0 )
            {
                return(LP_sendrawtransaction(coin,jstr(argjson,"signedtx")));
            }
            else if ( strcmp(method,"getrawtransaction") == 0 )
            {
                return(jprint(LP_gettx(coin,jbits256(argjson,"txid"),0),1));
            }
            else if ( strcmp(method,"withdraw") == 0 )
            {
                if ( (ptr= LP_coinsearch(coin)) != 0 )
                {
                    if ( jobj(argjson,"outputs") == 0 )
                        return(clonestr("{\"error\":\"withdraw needs to have outputs\"}"));
                    else return(LP_withdraw(ptr,argjson));
                }
                return(clonestr("{\"error\":\"cant find coind\"}"));
            }
            else if ( strcmp(method,"setconfirms") == 0 )
            {
                int32_t n;
                n = jint(argjson,"numconfirms");
                if ( n < 0 )
                    return(clonestr("{\"error\":\"illegal numconfirms\"}"));
                if ( (ptr= LP_coinsearch(coin)) != 0 )
                {
                    ptr->userconfirms = n;
                    if ( (n= jint(argjson,"maxconfirms")) > 0 )
                        ptr->maxconfirms = n;
                    if ( ptr->maxconfirms > 0 && ptr->userconfirms > ptr->maxconfirms )
                        ptr->userconfirms = ptr->maxconfirms;
                    return(clonestr("{\"result\":\"success\"}"));
                } else return(clonestr("{\"error\":\"cant find coind\"}"));
            }
            else if ( strcmp(method,"snapshot") == 0 )
            {
                if ( (ptr= LP_coinsearch(coin)) != 0 )
                    return(jprint(LP_snapshot(ptr,juint(argjson,"height")),1));
                else return(clonestr("{\"error\":\"cant find coind\"}"));
            }
            else if ( strcmp(method,"dividends") == 0 )
            {
                if ( (ptr= LP_coinsearch(coin)) != 0 )
                    return(LP_dividends(ptr,juint(argjson,"height"),argjson));
                else return(clonestr("{\"error\":\"cant find coind\"}"));
            }
            else if ( strcmp(method,"snapshot_balance") == 0 )
            {
                if ( (ptr= LP_coinsearch(coin)) != 0 )
                    return(LP_snapshot_balance(ptr,juint(argjson,"height"),argjson));
                else return(clonestr("{\"error\":\"cant find coind\"}"));
            }
            if ( LP_isdisabled(coin,0) != 0 )
            {
                retjson = cJSON_CreateObject();
                jaddstr(retjson,"error",LP_DONTCHANGE_ERRMSG1);
                return(jprint(retjson,1));
            }
            if ( strcmp(method,"inventory") == 0 )
            {
                struct iguana_info *ptr;
                if ( (ptr= LP_coinfind(coin)) != 0 )
                {
                    LP_address(ptr,ptr->smartaddr);
                    if ( jint(argjson,"reset") != 0 )
                    {
                        ptr->privkeydepth = 0;
                        LP_address_utxo_reset(ptr);
                        LP_passphrase_init(jstr(argjson,"passphrase"),G.gui,G.netid,G.seednode);
                    }
                    if ( bits256_nonz(G.LP_privkey) != 0 )
                        LP_privkey_init(-1,ptr,G.LP_privkey,G.LP_mypub25519);
                    else printf("no LP_privkey\n");
                    retjson = cJSON_CreateObject();
                    jaddstr(retjson,"result","success");
                    jaddstr(retjson,"coin",coin);
                    jaddnum(retjson,"timestamp",time(NULL));
                    jadd(retjson,"alice",cJSON_Parse("[]"));
                    //jadd(retjson,"alice",LP_inventory(coin));
                    //jadd(retjson,"bob",LP_inventory(coin,1));
                    //LP_smartutxos_push(ptr);
                    LP_address_utxo_reset(ptr);
                    return(jprint(retjson,1));
                }
            }
            else if ( strcmp(method,"goal") == 0 )
                return(LP_portfolio_goal(coin,jdouble(argjson,"val")));
            else if ( strcmp(method,"getcoin") == 0 )
                return(LP_getcoin(coin));
        }
        else if ( strcmp(method,"goal") == 0 )
            return(LP_portfolio_goal("*",100.));
        else if ( strcmp(method,"lastnonce") == 0 )
        {
            cJSON *retjson = cJSON_CreateObject();
            jaddstr(retjson,"result","success");
            jaddnum(retjson,"lastnonce",LP_lastnonce);
            return(jprint(retjson,1));
        }
        else if ( strcmp(method,"myprices") == 0 )
            return(LP_myprices());
        else if ( strcmp(method,"trust") == 0 )
            return(LP_pubkey_trustset(jbits256(argjson,"pubkey"),jint(argjson,"trust")));
        else if ( strcmp(method,"trusted") == 0 )
            return(LP_pubkey_trusted());
    } // end of protected localhost commands
    if ( IAMLP == 0 )
    {
        if ( (reqjson= LP_dereference(argjson,"broadcast")) != 0 )
        {
            if ( jobj(reqjson,"method2") != 0 )
            {
                jdelete(reqjson,"method");
                method = jstr(reqjson,"method2");
                jaddstr(reqjson,"method",method);
            }
            argjson = reqjson;
        }
        if ( strcmp(method,"getdPoW") == 0 )
            retstr = clonestr("{\"result\":\"success\"}");
    }
    else
    {
        if ( strcmp(method,"tradesarray") == 0 )
        {
            return(jprint(LP_tradesarray(base,rel,juint(argjson,"starttime"),juint(argjson,"endtime"),jint(argjson,"timescale")),1));
        }
        else if ( strcmp(method,"getdPoW") == 0 )
        {
            if ( (ptr= LP_coinfind(jstr(argjson,"coin"))) != 0 )
                LP_dPoW_broadcast(ptr);
            retstr = clonestr("{\"result\":\"success\"}");
        }
    }
    // received response
    if ( strcmp(method,"swapstatus") == 0 )
        return(LP_swapstatus_recv(argjson));
    else if ( strcmp(method,"gettradestatus") == 0 )
    {
        retstr = clonestr("{\"error\":\"deprecated\"}");
        //return(LP_gettradestatus(j64bits(argjson,"aliceid"),juint(argjson,"requestid"),juint(argjson,"quoteid")));
    }
    else if ( strcmp(method,"postprice") == 0 )
        return(LP_postprice_recv(argjson));
    else if ( strcmp(method,"uitem") == 0 )
        return(LP_uitem_recv(argjson));
    else if ( strcmp(method,"dPoW") == 0 )
        return(LP_dPoW_recv(argjson));
    else if ( strcmp(method,"notify") == 0 )
        return(LP_notify_recv(argjson));
    else if ( strcmp(method,"getpeers") == 0 )
        return(LP_peers());
    else if ( strcmp(method,"balances") == 0 )
        return(jprint(LP_balances(jstr(argjson,"address")),1));
    else if ( strcmp(method,"fundvalue") == 0 )
        return(jprint(LP_fundvalue(argjson),1));
    else if ( strcmp(method,"getprice") == 0 || strcmp(method,"getmyprice") == 0 )
    {
        double price,bid,ask;
        if ( strcmp(method,"getprice") == 0 )
        {
            ask = LP_price(base,rel);
            if ( (bid= LP_price(rel,base)) > SMALLVAL )
                bid = 1./bid;
        }
        else
        {
            ask = LP_getmyprice(base,rel);
            if ( (bid= LP_getmyprice(rel,base)) > SMALLVAL )
                bid = 1./bid;
        }
        price = _pairaved(bid,ask);
        retjson = cJSON_CreateObject();
        jaddstr(retjson,"result","success");
        jaddstr(retjson,"base",base);
        jaddstr(retjson,"rel",rel);
        jaddnum(retjson,"timestamp",time(NULL));
        jaddnum(retjson,"bid",bid);
        jaddnum(retjson,"ask",ask);
        jaddnum(retjson,"price",price);
        return(jprint(retjson,1));
    }
    /*else if ( strcmp(method,"getpeers") == 0 )
    {
        char *tmpstr;
        if ( (tmpstr= jstr(argjson,"LPnode")) != 0 )
            LP_addpeer(LP_mypeer,LP_mypubsock,tmpstr,RPC_port,RPC_port+10,RPC_port+20,1,G.LP_sessionid);
        if ( IAMLP != 0 )
        {
            printf("send peers list %s\n",LP_peers());
            bits256 zero; memset(zero.bytes,0,sizeof(zero));
            LP_reserved_msg(0,"","",zero,LP_peers());
        }
        retstr = clonestr("{\"result\":\"success\"}");
    }*/
    // end received response
    
    else if ( strcmp(method,"tradestatus") == 0 )
    {
        LP_tradecommand_log(argjson);
        //printf("%-4d tradestatus | aliceid.%llu RT.%d %d\n",(uint32_t)time(NULL) % 3600,(long long)j64bits(argjson,"aliceid"),LP_RTcount,LP_swapscount);
        retstr = clonestr("{\"result\":\"success\"}");
    }
    else if ( strcmp(method,"wantnotify") == 0 )
    {
        bits256 pub; static uint32_t lastnotify;
        pub = jbits256(argjson,"pub");
        //char str[65]; printf("got wantnotify.(%s) vs %s\n",jprint(argjson,0),bits256_str(str,G.LP_mypub25519));
        if ( bits256_cmp(pub,G.LP_mypub25519) == 0 && time(NULL) > lastnotify+60 )
        {
            lastnotify = (uint32_t)time(NULL);
            //printf("wantnotify for me!\n");
            LP_notify_pubkeys(ctx,LP_mypubsock);
        }
        retstr = clonestr("{\"result\":\"success\"}");
    }
    else if ( strcmp(method,"addr_unspents") == 0 )
    {
        //printf("GOT ADDR_UNSPENTS %s %s\n",jstr(argjson,"coin"),jstr(argjson,"address"));
        if ( (ptr= LP_coinsearch(coin)) != 0 )
        {
            char *coinaddr;
            if ( (coinaddr= jstr(argjson,"address")) != 0 )
            {
                if ( coinaddr[0] != 0 )
                {
                    LP_address(ptr,coinaddr);
                    if ( strcmp(coinaddr,ptr->smartaddr) == 0 && bits256_nonz(G.LP_privkey) != 0 )
                    {
                        //printf("ADDR_UNSPENTS %s %s is my address being asked for!\n",ptr->symbol,coinaddr);
                        if ( ptr->lastpushtime > 0 && ptr->addr_listunspent_requested > (uint32_t)time(NULL)-10 )
                            ptr->lastpushtime -= LP_ORDERBOOK_DURATION*0.1;
                        ptr->addr_listunspent_requested = (uint32_t)time(NULL);
                    }
                }
            }
        }
        retstr = clonestr("{\"result\":\"success\"}");
    }
    else if ( strcmp(method,"encrypted") == 0 )
        retstr = clonestr("{\"result\":\"success\"}");
    else // psock requests/response
    {
        if ( IAMLP != 0 )
        {
            if ( strcmp(method,"psock") == 0 )
            {
                int32_t psock;
                if ( myipaddr == 0 || myipaddr[0] == 0 || strcmp(myipaddr,"127.0.0.1") == 0 )
                {
                    if ( LP_mypeer != 0 )
                        myipaddr = LP_mypeer->ipaddr;
                    else printf("LP_psock dont have actual ipaddr?\n");
                }
                if ( jint(argjson,"ispaired") != 0 )
                {
                    retstr = LP_psock(&psock,myipaddr,1,jint(argjson,"cmdchannel"),jbits256(argjson,"pubkey"));
                    //printf("LP_commands.(%s)\n",retstr);
                    return(retstr);
                }
                else return(clonestr("{\"error\":\"you are running an obsolete version, update\"}"));
            }
        }
        else
        {
            if ( strcmp(method,"psock") == 0 )
            {
                //printf("nonLP got (%s)\n",jprint(argjson,0));
                retstr = clonestr("{\"result\":\"success\"}");
            }
        }
    }
    if ( retstr == 0 )
        printf("ERROR.(%s)\n",jprint(argjson,0));
    if ( reqjson != 0 )
        free_json(reqjson);
    if ( retstr != 0 )
    {
        free(retstr);
        return(0);
    }
    return(0);
}
