
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
//  LP_utxos.c
//  marketmaker
//

int32_t LP_privkey_init(int32_t mypubsock,struct iguana_info *coin,bits256 myprivkey,bits256 mypub)
{
    int32_t enable_utxos = 0;
    char *script,destaddr[64]; cJSON *array,*item; bits256 txid,deposittxid,zero; int32_t used,i,flag=0,height,n,cmpflag,iambob,vout,depositvout; uint64_t *values=0,satoshis,txfee,biggerval,value,total = 0; int64_t targetval; //struct LP_utxoinfo *utxo;
    if ( coin == 0 || (IAMLP == 0 && coin->inactive != 0) )
    {
        //printf("coin not active\n");
        return(0);
    }
    if ( coin->privkeydepth > 0 )
        return(0);
    coin->privkeydepth++;
    LP_address(coin,coin->smartaddr);
    //if ( coin->inactive == 0 )
    //    LP_listunspent_issue(coin->symbol,coin->smartaddr,0);
    memset(zero.bytes,0,sizeof(zero));
    array = LP_listunspent(coin->symbol,coin->smartaddr,zero,zero);
    if ( array != 0 )
    {
        txfee = LP_txfeecalc(coin,0,0);
        if ( is_cJSON_Array(array) != 0 && (n= cJSON_GetArraySize(array)) > 0 )
        {
            coin->numutxos = n;
            //printf("LP_privkey_init %s %d\n",coin->symbol,n);
            for (iambob=0; iambob<=1; iambob++)
            {
                if ( iambob == 0 )
                    values = calloc(n,sizeof(*values));
                else memset(values,0,n * sizeof(*values));
                used = 0;
                for (i=0; i<n; i++)
                {
                    item = jitem(array,i);
                    value = LP_listunspent_parseitem(coin,&txid,&vout,&height,item);
                    satoshis = LP_txvalue(destaddr,coin->symbol,txid,vout);
                    if ( satoshis != 0 && satoshis != value )
                        printf("%s %s  privkey_init value  %.8f vs %.8f (%s) %.8f %.8f\n",coin->symbol,coin->smartaddr,dstr(satoshis),dstr(value),jprint(item,0),jdouble(item,"amount"),jdouble(item,"interest"));
                    if ( coin->electrum != 0 || LP_inventory_prevent(iambob,coin->symbol,txid,vout) == 0 )//&& height > 0 )
                    {
                        values[i] = satoshis;
                        //flag += LP_address_utxoadd(coin,destaddr,txid,vout,satoshis,height,-1);
                    } else used++;
                }
                //printf("array.%d\n",n);
                while ( used < n-1 )
                {
                    //for (i=0; i<n; i++)
                    //   printf("%.8f ",dstr(values[i]));
                    //printf("used.%d of n.%d\n",used,n);
                    if ( (i= LP_maxvalue(values,n)) >= 0 )
                    {
                        item = jitem(array,i);
                        if ( coin->electrum == 0 )
                        {
                            deposittxid = jbits256(item,"txid");
                            depositvout = juint(item,"vout");
                            script = jstr(item,"scriptPubKey");
                        }
                        else
                        {
                            deposittxid = jbits256(item,"tx_hash");
                            depositvout = juint(item,"tx_pos");
                            script = coin->smartaddr;
                        }
                        biggerval = values[i];
                        values[i] = 0, used++;
                        if ( iambob == 0 )
                            targetval = (biggerval / 776) + txfee;
                        else targetval = (biggerval / 9) * 8 + 2*txfee;
                        if ( targetval < txfee*2 )
                            targetval = txfee*2;
                        //printf("iambob.%d i.%d deposit %.8f min %.8f target %.8f\n",iambob,i,dstr(biggerval),dstr((1+LP_MINSIZE_TXFEEMULT)*txfee),dstr(targetval));
                        if ( biggerval < (1+LP_MINSIZE_TXFEEMULT)*txfee )
                            continue;
                        i = -1;
                        if ( iambob != 0 )
                        {
                            if ( (i= LP_nearestvalue(iambob,values,n,targetval)) < 0 )
                                targetval /= 4;
                            if ( targetval < txfee*(1+LP_MINSIZE_TXFEEMULT) )
                                continue;
                        }
                        if ( i >= 0 || (i= LP_nearestvalue(iambob,values,n,targetval)) >= 0 )
                        {
                            //printf("iambob.%d i.%d %.8f target %.8f\n",iambob,i,dstr(biggerval),dstr(targetval));
                            item = jitem(array,i);
                            cmpflag = 0;
                            if ( coin->electrum == 0 )
                            {
                                txid = jbits256(item,"txid");
                                vout = juint(item,"vout");
                                if ( jstr(item,"scriptPubKey") != 0 && strcmp(script,jstr(item,"scriptPubKey")) == 0 )
                                    cmpflag = 1;
                            }
                            else
                            {
                                txid = jbits256(item,"tx_hash");
                                vout = juint(item,"tx_pos");
                                cmpflag = 1;
                            }
                            if ( cmpflag != 0 )
                            {
                                value = values[i];
                                values[i] = 0, used++;
                                /*portable_mutex_lock(&LP_UTXOmutex);
                                if ( iambob != 0 )
                                {
                                    if ( (utxo= LP_utxoadd(1,coin->symbol,txid,vout,value,deposittxid,depositvout,biggerval,coin->smartaddr,mypub,LP_gui,G.LP_sessionid,value)) != 0 )
                                    {
                                    }
                                }
                                else
                                {
                                    //printf("call utxoadd\n");
                                    if ( (utxo= LP_utxoadd(0,coin->symbol,deposittxid,depositvout,biggerval,txid,vout,value,coin->smartaddr,mypub,LP_gui,G.LP_sessionid,biggerval)) != 0 )
                                    {
                                    }
                                }
                                portable_mutex_unlock(&LP_UTXOmutex);*/
                                total += value;
                            } // else printf("scriptmismatch.(%s) vs %s\n",script,jprint(item,0));
                        } //else printf("nothing near i.%d\n",i);
                    } else break;
                }
                if ( enable_utxos == 0 )
                    break;
            }
        }
        free_json(array);
        if ( 0 && flag != 0 )
            LP_postutxos(coin->symbol,coin->smartaddr);
    }
    if ( values != 0 )
        free(values);
    if ( coin->privkeydepth > 0 )
        coin->privkeydepth--;
    //printf("privkey.%s %.8f\n",symbol,dstr(total));
    return(flag);
}

char *LP_secretaddresses(void *ctx,char *prefix,char *passphrase,int32_t n,uint8_t taddr,uint8_t pubtype)
{
    int32_t i; uint8_t tmptype,pubkey33[33],rmd160[20]; char output[777*45],str[65],str2[65],buf[8192],wifstr[128],coinaddr[64]; bits256 checkprivkey,privkey,pubkey; cJSON *retjson;
    retjson = cJSON_CreateObject();
    if ( prefix == 0 || prefix[0] == 0 )
        prefix = "secretaddress";
    if ( passphrase == 0 || passphrase[0] == 0 )
        passphrase = "password";
    if ( n <= 0 )
        n = 16;
    else if ( n > 777 )
        n = 777;
    conv_NXTpassword(privkey.bytes,pubkey.bytes,(uint8_t *)passphrase,(int32_t)strlen(passphrase));
    bitcoin_priv2pub(ctx,"KMD",pubkey33,coinaddr,privkey,taddr,pubtype);
    printf("generator (%s) secrets.[%d] <%s> t.%u p.%u\n",coinaddr,n,passphrase,taddr,pubtype);
    sprintf(output,"\"addresses\":[");
    for (i=0; i<n; i++)
    {
        sprintf(buf,"%s %s %03d",prefix,passphrase,i);
        conv_NXTpassword(privkey.bytes,pubkey.bytes,(uint8_t *)buf,(int32_t)strlen(buf));
        bitcoin_priv2pub(ctx,"KMD",pubkey33,coinaddr,privkey,taddr,pubtype);
        bitcoin_priv2wif("KMD",0,wifstr,privkey,188);
        bitcoin_wif2priv("KMD",0,&tmptype,&checkprivkey,wifstr);
        bitcoin_addr2rmd160("KMD",taddr,&tmptype,rmd160,coinaddr);
        if ( bits256_cmp(checkprivkey,privkey) != 0 )
        {
            printf("WIF.(%s) error -> %s vs %s?\n",wifstr,bits256_str(str,privkey),bits256_str(str2,checkprivkey));
            free_json(retjson);
            return(clonestr("{\"error\":\"couldnt validate wifstr\"}"));
        }
        else if ( tmptype != pubtype )
        {
            printf("checktype.%d != pubtype.%d\n",tmptype,pubtype);
            free_json(retjson);
            return(clonestr("{\"error\":\"couldnt validate pubtype\"}"));
        }
        jaddstr(retjson,coinaddr,wifstr);
        sprintf(output+strlen(output),"\\\"%s\\\"%c ",coinaddr,i<n-1?',':' ');
        printf("./komodo-cli jumblr_secret %s\n",coinaddr);
    }
    printf("%s]\n",output);
    return(jprint(retjson,1));
}

static const char base58_chars[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

int32_t LP_wifstr_valid(char *symbol,char *wifstr)
{
    bits256 privkey,cmpkey; uint8_t wiftype; char cmpstr[128],cmpstr2[128]; int32_t i,len,n,a,A;
    if ( (len= (int32_t)strlen(wifstr)) < 50 || len > 54 )
    {
        //printf("len.%d is wrong for wif %s\n",len,wifstr);
        return(0);
    }
    memset(privkey.bytes,0,sizeof(privkey));
    memset(cmpkey.bytes,0,sizeof(cmpkey));
    for (i=n=a=A=0; wifstr[i]!=0; i++)
    {
        if ( strchr(base58_chars,wifstr[i]) == 0 )
            return(0);
        if ( wifstr[i] >= '1' && wifstr[i] <= '9' )
            n++;
        else if ( wifstr[i] >= 'A' && wifstr[i] <= 'Z' )
            A++;
        else if ( wifstr[i] >= 'a' && wifstr[i] <= 'z' )
            a++;
    }
    if ( n == 0 || A == 0 || a == 0 )
        return(0);
    if ( A > 5*a || a > 5*A || a > n*20 || A > n*20 ) // unlikely it is a real wif
    {
        printf("reject wif %s due to n.%d a.%d A.%d (%d %d %d %d)\n",wifstr,n,a,A,A > 5*a,a < 5*A,a > n*20,A > n*20);
        return(0);
    }
    bitcoin_wif2priv(symbol,0,&wiftype,&privkey,wifstr);
    bitcoin_priv2wif(symbol,0,cmpstr,privkey,wiftype);
    if ( strcmp(cmpstr,wifstr) == 0 )
    {
        //printf("%s is valid wif\n",wifstr);
        return(1);
    }
    else if ( bits256_nonz(privkey) != 0 )
    {
        bitcoin_wif2priv(symbol,0,&wiftype,&cmpkey,cmpstr);
        bitcoin_priv2wiflong(symbol,0,cmpstr2,privkey,wiftype);
        if ( bits256_cmp(privkey,cmpkey) == 0 )
            return(1);
        char str[65],str2[65]; printf("%s mismatched wifstr %s -> %s -> %s %s %s\n",symbol,wifstr,bits256_str(str,privkey),cmpstr,bits256_str(str2,cmpkey),cmpstr2);
    }
    char str[65]; printf("%s is not a wif, privkey.%s\n",wifstr,bits256_str(str,privkey));
    return(0);
}

bits256 LP_privkeycalc(void *ctx,uint8_t *pubkey33,bits256 *pubkeyp,struct iguana_info *coin,char *passphrase,char *wifstr)
{
    //static uint32_t counter;
    bits256 privkey,userpub,zero,userpass,checkkey,tmpkey; char str[65],str2[65],tmpstr[128]; cJSON *retjson; uint8_t tmptype,sig[128]; int32_t notarized,siglen; uint64_t nxtaddr;
    if ( (wifstr == 0 || wifstr[0] == 0) && LP_wifstr_valid(coin->symbol,passphrase) > 0 )
    {
        wifstr = passphrase;
        passphrase = 0;
    }
    if ( passphrase != 0 && passphrase[0] != 0 )
    {
        calc_NXTaddr(G.LP_NXTaddr,userpub.bytes,(uint8_t *)passphrase,(int32_t)strlen(passphrase));
        conv_NXTpassword(privkey.bytes,pubkeyp->bytes,(uint8_t *)passphrase,(int32_t)strlen(passphrase));
        privkey.bytes[0] &= 248, privkey.bytes[31] &= 127, privkey.bytes[31] |= 64;
        //vcalc_sha256(0,checkkey.bytes,(uint8_t *)passphrase,(int32_t)strlen(passphrase));
        //printf("SHA256.(%s) ",bits256_str(pstr,checkkey));
        //printf("privkey.(%s)\n",bits256_str(pstr,privkey));
        bitcoin_priv2wif(coin->symbol,coin->wiftaddr,tmpstr,privkey,coin->wiftype);
        bitcoin_wif2priv(coin->symbol,coin->wiftaddr,&tmptype,&checkkey,tmpstr);
        if ( bits256_cmp(privkey,checkkey) != 0 )
        {
            char str[65],str2[65]; printf("mismatched privkeys from wif conversion: %s -> %s -> %s\n",bits256_str(str,privkey),tmpstr,bits256_str(str2,checkkey));
            exit(1);
        }
    }
    else
    {
        bitcoin_wif2priv(coin->symbol,coin->wiftaddr,&tmptype,&privkey,wifstr);
        bitcoin_priv2wif(coin->symbol,coin->wiftaddr,tmpstr,privkey,tmptype);
        if ( strcmp(tmpstr,wifstr) != 0 )
        {
            bitcoin_wif2priv(coin->symbol,coin->wiftaddr,&tmptype,&tmpkey,tmpstr);
            if ( bits256_cmp(tmpkey,privkey) != 0 )
            {
                char str[65]; printf("%s error reproducing the wifstr, likely edge case like non-supported uncompressed pubkey privkey.%s\n",coin->symbol,bits256_str(str,privkey));
                exit(1);
            }
        }
        tmpkey = privkey;
        nxtaddr = conv_NXTpassword(tmpkey.bytes,pubkeyp->bytes,0,0);
        RS_encode(G.LP_NXTaddr,nxtaddr);
    }
    bitcoin_priv2pub(ctx,coin->symbol,coin->pubkey33,coin->smartaddr,privkey,coin->taddr,coin->pubtype);
    OS_randombytes(tmpkey.bytes,sizeof(tmpkey));
    if ( bits256_nonz(privkey) == 0 || (siglen= bitcoin_sign(ctx,coin->symbol,sig,tmpkey,privkey,0)) <= 0 )
    {
        printf("illegal privkey %s\n",bits256_str(str,privkey));
        exit(0);
    }
    if ( bitcoin_verify(ctx,sig,siglen,tmpkey,coin->pubkey33,33) != 0 )
    {
        printf("signature.[%d] for %s by %s didnt verify\n",siglen,bits256_str(str,tmpkey),bits256_str(str2,privkey));
        exit(0);
    }
    if ( coin->counter == 0 )
    {
        coin->counter++;
        memcpy(G.LP_pubsecp,coin->pubkey33,33);
        bitcoin_priv2wif(coin->symbol,coin->wiftaddr,tmpstr,privkey,coin->wiftype);
        bitcoin_addr2rmd160(coin->symbol,coin->taddr,&tmptype,G.LP_myrmd160,coin->smartaddr);
        LP_privkeyadd(privkey,G.LP_myrmd160);
        G.LP_privkey = privkey;
        if ( G.counter++ == 0 )
        {
            bitcoin_priv2wif(coin->symbol,coin->wiftaddr,G.USERPASS_WIFSTR,privkey,188);
            bitcoin_wif2priv(coin->symbol,coin->wiftaddr,&tmptype,&checkkey,G.USERPASS_WIFSTR);
            if ( bits256_cmp(checkkey,privkey) != 0 )
            {
                char str[65],str2[65];
                printf("FATAL ERROR converting USERPASS_WIFSTR %s -> %s != %s\n",G.USERPASS_WIFSTR,bits256_str(str,checkkey),bits256_str(str2,privkey));
                exit(-1);
            }
            conv_NXTpassword(userpass.bytes,pubkeyp->bytes,(uint8_t *)G.USERPASS_WIFSTR,(int32_t)strlen(G.USERPASS_WIFSTR));
            userpub = curve25519(userpass,curve25519_basepoint9());
            printf("userpass.(%s)\n",bits256_str(G.USERPASS,userpub));
        }
    }
    if ( strcmp(coin->smartaddr,"RPZVpjptzfZnFZZoLnuSbfLexjtkhe6uvn") != 0 && coin->importedprivkey == 0 && coin->electrum == 0 && coin->userpass[0] != 0 && LP_getheight(&notarized,coin) > 0 )
    {
        memset(zero.bytes,0,sizeof(zero));
        LP_listunspent_issue(coin->symbol,coin->smartaddr,0,zero,zero);
        if ( (retjson= LP_importprivkey(coin->symbol,tmpstr,coin->smartaddr,-1)) != 0 )
        {
            if ( jobj(retjson,"error") != 0 )
            {
                printf("cant importprivkey.%s %s -> (%s), abort session\n",coin->symbol,coin->smartaddr,jprint(retjson,1));
                exit(-1);
            }
            free_json(retjson);
        }
        coin->importedprivkey = (uint32_t)time(NULL);
    }
    vcalc_sha256(0,checkkey.bytes,privkey.bytes,sizeof(privkey));
    checkkey.bytes[0] &= 248, checkkey.bytes[31] &= 127, checkkey.bytes[31] |= 64;
    G.LP_mypub25519 = *pubkeyp = curve25519(checkkey,curve25519_basepoint9());
    G.LP_mypriv25519 = checkkey;
    LP_pubkeyadd(G.LP_mypub25519);
    return(privkey);
}

void LP_privkey_updates(void *ctx,int32_t pubsock,char *passphrase)
{
    struct iguana_info *coin,*tmp; bits256 pubkey,privkey; uint8_t pubkey33[33]; int32_t initonly;
    initonly = (passphrase != 0);
    memset(privkey.bytes,0,sizeof(privkey));
    memset(pubkey.bytes,0,sizeof(pubkey));
	//printf("Total coins: %d\n", HASH_COUNT(LP_coins));
	//int num_iter = 0;
    HASH_ITER(hh,LP_coins,coin,tmp)
    {
		//printf("LP_privkey_updates [%02d / %02d]\n", num_iter++, HASH_COUNT(LP_coins));
        if ( initonly != 0 )
        {
            coin->counter = 0;
            memset(coin->smartaddr,0,sizeof(coin->smartaddr));
            if ( bits256_nonz(privkey) == 0 || coin->smartaddr[0] == 0 )
                privkey = LP_privkeycalc(ctx,pubkey33,&pubkey,coin,passphrase,"");
        }
        //printf("i.%d of %d\n",i,LP_numcoins);
        else if ( IAMLP == 0 || coin->inactive == 0 )
        {
            //printf("from updates %s\n",coin->symbol);
            if ( 0 && LP_privkey_init(pubsock,coin,G.LP_privkey,G.LP_mypub25519) == 0 && (LP_rand() % 10) == 0 )
            {
                //LP_postutxos(coin->symbol,coin->smartaddr);
            }
        }
    }
}

int32_t LP_passphrase_init(char *passphrase,char *gui,uint16_t netid,char *seednode)
{
    static void *ctx; struct iguana_info *coin,*tmp; int32_t counter;
    if ( ctx == 0 )
        ctx = bitcoin_ctx();
    if ( G.LP_pendingswaps != 0 )
        return(-1);
    if ( netid != G.netid )
    {
        if ( IAMLP != 0 )
        {
            printf("sorry, LP nodes can only set netid during startup\n");
            return(-1);
        }
        else
        {
            printf(">>>>>>>>>>>>> netid.%d vs G.netid %d\n",netid,G.netid);
            LP_closepeers();
            LP_initpeers(LP_mypubsock,LP_mypeer,LP_myipaddr,RPC_port,netid,seednode);
        }
    }
    G.initializing = 1;
    if ( gui == 0 )
        gui = "cli";
    counter = G.USERPASS_COUNTER;
    HASH_ITER(hh,LP_coins,coin,tmp)
    {
        coin->importedprivkey = 0;
    }
    while ( G.waiting == 0 )
    {
        printf("waiting for G.waiting\n");
        sleep(5);
    }
    memset(&G,0,sizeof(G));
    G.netid = netid;
    safecopy(G.seednode,seednode,sizeof(G.seednode));
    vcalc_sha256(0,G.LP_passhash.bytes,(uint8_t *)passphrase,(int32_t)strlen(passphrase));
    LP_privkey_updates(ctx,LP_mypubsock,passphrase);
    init_hexbytes_noT(G.LP_myrmd160str,G.LP_myrmd160,20);
    G.LP_sessionid = (uint32_t)time(NULL);
    safecopy(G.gui,gui,sizeof(G.gui));
    LP_tradebot_pauseall();
    LP_portfolio_reset();
    LP_priceinfos_clear();
    G.USERPASS_COUNTER = counter;
    G.initializing = 0;
    //LP_cmdchannels();
    return(0);
}

void LP_privkey_tests()
{
    char wifstr[64],str[65],str2[65]; bits256 privkey,checkkey; int32_t i; uint8_t tmptype;
    for (i=0; i<200000000; i++)
    {
        privkey = rand256(0);
        bitcoin_priv2wif("KMD",0,wifstr,privkey,0xff);
        bitcoin_wif2priv("KMD",0,&tmptype,&checkkey,wifstr);
        if ( bits256_cmp(privkey,checkkey) != 0 )
        {
            printf("i.%d: %s vs %s\n",i,bits256_str(str,privkey),bits256_str(str2,checkkey));
            exit(-1);
        }
        if ( (i % 1000000) == 0 )
            fprintf(stderr,"%.1f%% ",100.*(double)i/200000000);
    }
    printf("%d privkeys checked\n",i);
}


#define JPG_ENCRYPTED_MAXSIZE 32768

int32_t JPG_encrypt(uint16_t ind,uint8_t encoded[JPG_ENCRYPTED_MAXSIZE],uint8_t *msg,int32_t msglen,bits256 privkey)
{
    bits256 pubkey; int32_t len = 2; uint8_t space[JPG_ENCRYPTED_MAXSIZE],*nonce,*cipher;
    pubkey = acct777_pubkey(privkey);
    encoded[len++] = ind & 0xff;
    encoded[len++] = (ind >> 8) & 0xff;
    nonce = &encoded[len];
    OS_randombytes(nonce,crypto_box_NONCEBYTES);
    cipher = &encoded[len + crypto_box_NONCEBYTES];
    msglen = _SuperNET_cipher(nonce,&encoded[len + crypto_box_NONCEBYTES],msg,msglen,pubkey,privkey,space);
    msglen += crypto_box_NONCEBYTES;
    msg = encoded;
    msglen += len;
    encoded[0] = msglen & 0xff;
    encoded[1] = (msglen >> 8) & 0xff;
    int32_t i; for (i=0; i<msglen; i++)
        printf("%02x",encoded[i]);
    printf(" encoded.%d\n",msglen);
   return(msglen);
}

uint8_t *JPG_decrypt(uint16_t *indp,int32_t *recvlenp,uint8_t space[JPG_ENCRYPTED_MAXSIZE + crypto_box_ZEROBYTES],uint8_t *encoded,bits256 privkey)
{
    bits256 pubkey; uint8_t *extracted=0,*nonce,*cipher; uint16_t msglen,ind; int32_t cipherlen,len = 4;
    *recvlenp = 0;
    *indp = -1;
    pubkey = acct777_pubkey(privkey);
    msglen = ((int32_t)encoded[1] << 8) | encoded[0];
    ind = ((int32_t)encoded[3] << 8) | encoded[2];
    nonce = &encoded[len];
    cipher = &encoded[len + crypto_box_NONCEBYTES];
    cipherlen = msglen - (len + crypto_box_NONCEBYTES);
    if ( cipherlen > 0 && cipherlen <= JPG_ENCRYPTED_MAXSIZE + crypto_box_ZEROBYTES )
    {
        if ( (extracted= _SuperNET_decipher(nonce,cipher,space,cipherlen,pubkey,privkey)) != 0 )
        {
            int32_t i; for (i=0; i<msglen; i++)
                printf("%02x",encoded[i]);
            printf(" restored\n");
            msglen = (cipherlen - crypto_box_ZEROBYTES);
            *recvlenp = msglen;
            *indp = ind;
        }
    } //else printf("cipher.%d too big for %d\n",cipherlen,JPG_ENCRYPTED_MAXSIZE + crypto_box_ZEROBYTES);
    return(extracted);
}

// from https://github.com/owencm/C-Steganography-Framework
#include "../../crypto777/jpeg/cdjpeg.h" // Common decls for compressing and decompressing jpegs

int32_t LP_jpg_process(int32_t *capacityp,char *inputfname,char *outputfname,uint8_t *decoded,uint8_t *origdata,int32_t origrequired,int32_t power2,char *password,uint16_t *indp)
{
    struct jpeg_decompress_struct inputinfo;
    struct jpeg_compress_struct outputinfo;
    struct jpeg_error_mgr jerr;
    jvirt_barray_ptr *coef_arrays;
    JDIMENSION i,compnum,rownum,blocknum;
    JBLOCKARRAY coef_buffers[MAX_COMPONENTS];
    JBLOCKARRAY row_ptrs[MAX_COMPONENTS];
    bits256 privkey; FILE *input_file,*output_file; int32_t recvlen,msglen,val,modified,emit,totalrows,limit,required; uint16_t checkind; uint8_t *decrypted,*space,*data=0;
    if ((input_file = fopen(inputfname, READ_BINARY)) == NULL)
    {
        fprintf(stderr, "Can't open %s\n", inputfname);
        //exit(EXIT_FAILURE);
        return(-1);
    }
    required = origrequired;
    memset(privkey.bytes,0,sizeof(privkey));
    if ( password != 0 && password[0] != 0 )
    {
        if ( required/8 > JPG_ENCRYPTED_MAXSIZE-60 )
            return(-1);
        data = calloc(1,required/8+512);
        vcalc_sha256(0,privkey.bytes,(uint8_t *)password,(int32_t)strlen(password));
        if ( origdata != 0 )
        {
            msglen = JPG_encrypt(*indp,data,origdata,required/8,privkey);
            required = msglen * 8;
            {
                space = calloc(1,JPG_ENCRYPTED_MAXSIZE);
                if ( (decrypted= JPG_decrypt(&checkind,&recvlen,space,data,privkey)) == 0 || recvlen != origrequired/8 || checkind != *indp || memcmp(decrypted,origdata,origrequired/8) != 0 )
                    printf("A decryption error: checkind.%d vs %d, recvlen.%d vs %d, decrypted.%p\n",checkind,*indp,recvlen,origrequired/8,decrypted);
                else
                {
                    for (i=0; i<recvlen; i++)
                        printf("%02x",decrypted[i]);
                    printf(" VERIFIED decryption.%d ind.%d msglen.%d required.%d\n",recvlen,*indp,msglen,required);
                }
                free(space);
            }
        } else required += 60 * 8;
    } else data = origdata;
    if ( power2 < 0 || power2 > 30 )
        power2 = 7;
    limit = 1;
    while ( power2 > 0 )
    {
        limit <<= 1;
        power2--;
    }
    // Initialize the JPEG compression and decompression objects with default error handling
    inputinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&inputinfo);
    // Specify data source for decompression and recompression
    jpeg_stdio_src(&inputinfo, input_file);
    (void) jpeg_read_header(&inputinfo, TRUE);
    for (compnum=0; compnum<inputinfo.num_components; compnum++)
        coef_buffers[compnum] = ((&inputinfo)->mem->alloc_barray)((j_common_ptr)&inputinfo,JPOOL_IMAGE,inputinfo.comp_info[compnum].width_in_blocks,inputinfo.comp_info[compnum].height_in_blocks);
    coef_arrays = jpeg_read_coefficients(&inputinfo);
    // Copy DCT coeffs to a new array
    int num_components = inputinfo.num_components;
    size_t *block_row_size;//[num_components];
    int *width_in_blocks;//[num_components];
    int *height_in_blocks;//[num_components];
    block_row_size = calloc(sizeof(*block_row_size),num_components);
    width_in_blocks = calloc(sizeof(*width_in_blocks),num_components);
    height_in_blocks = calloc(sizeof(*height_in_blocks),num_components);
    *capacityp = modified = emit = totalrows = 0;
    if ( decoded != 0 )
        memset(decoded,0,required/8+1);
    for (compnum=0; compnum<num_components; compnum++)
    {
        height_in_blocks[compnum] = inputinfo.comp_info[compnum].height_in_blocks;
        width_in_blocks[compnum] = inputinfo.comp_info[compnum].width_in_blocks;
        block_row_size[compnum] = (size_t) SIZEOF(JCOEF)*DCTSIZE2*width_in_blocks[compnum];
        for (rownum=0; rownum<height_in_blocks[compnum]; rownum++)
        {
            row_ptrs[compnum] = ((&inputinfo)->mem->access_virt_barray)((j_common_ptr)&inputinfo,coef_arrays[compnum],rownum,(JDIMENSION)1,FALSE);
            for (blocknum=0; blocknum<width_in_blocks[compnum]; blocknum++)
            {
                for (i=0; i<DCTSIZE2; i++)
                {
                    val = row_ptrs[compnum][0][blocknum][i];
                    if ( val < -limit || val >= limit )
                    {
                        if ( (*capacityp) < required )
                        {
                            if ( (val & 1) != 0 )
                                SETBIT(decoded,(*capacityp));
                            //printf("%c",(val&1)!=0?'1':'0');
                        }
                        (*capacityp)++;
                    }
                    coef_buffers[compnum][rownum][blocknum][i] = val;
                }
            }
        }
    }
    if ( password != 0 && password[0] != 0 )
    {
        space = calloc(1,JPG_ENCRYPTED_MAXSIZE);
        if ( (decrypted= JPG_decrypt(indp,&recvlen,space,decoded,privkey)) != 0 && recvlen == origrequired/8 )
        {
            for (i=0; i<recvlen; i++)
            {
                //printf("%02x",decrypted[i]);
                decoded[i] = decrypted[i];
            }
            //printf(" decrypted.%d ind.%d\n",recvlen,*indp);
        }
        free(space);
    }
   //printf(" capacity %d required.%d power2.%d limit.%d\n",*capacityp,required,power2,limit);
    if ( *capacityp > required && outputfname != 0 && outputfname[0] != 0 )
    {
        if ((output_file = fopen(outputfname, WRITE_BINARY)) == NULL) {
            fprintf(stderr, "Can't open %s\n", outputfname);
            if ( data != origdata )
                free(data);
            return(-1);
        }
        outputinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&outputinfo);
        jpeg_stdio_dest(&outputinfo, output_file);
        jpeg_copy_critical_parameters(&inputinfo,&outputinfo);
        // Print out or modify DCT coefficients
        for (compnum=0; compnum<num_components; compnum++)
        {
            for (rownum=0; rownum<height_in_blocks[compnum]; rownum++)
            {
                for (blocknum=0; blocknum<width_in_blocks[compnum]; blocknum++)
                {
                    //printf("\n\nComponent: %i, Row:%i, Column: %i\n", compnum, rownum, blocknum);
                    for (i=0; i<DCTSIZE2&&emit<required; i++)
                    {
                        val = coef_buffers[compnum][rownum][blocknum][i];
                        if ( val < -limit || val >= limit )
                        {
                            val &= ~1;
                            if (GETBIT(data,emit) != 0 )//|| (emit >= required && (rand() & 1) != 0) )
                                val |= 1;
                            //printf("%c",(val&1)!=0?'1':'0');
                            coef_buffers[compnum][rownum][blocknum][i] = val;
                            emit++;
                        }
                        //printf("%i,", coef_buffers[compnum][rownum][blocknum][i]);
                    }
                }
            }
        }
        //printf(" emit.%d\n",emit);
        // Output the new DCT coeffs to a JPEG file
        modified = 0;
        for (compnum=0; compnum<num_components; compnum++)
        {
            for (rownum=0; rownum<height_in_blocks[compnum]; rownum++)
            {
                row_ptrs[compnum] = ((&outputinfo)->mem->access_virt_barray)((j_common_ptr)&outputinfo,coef_arrays[compnum],rownum,(JDIMENSION)1,TRUE);
                if ( memcmp(row_ptrs[compnum][0][0],coef_buffers[compnum][rownum][0],block_row_size[compnum]) != 0 )
                {
                    memcpy(row_ptrs[compnum][0][0],coef_buffers[compnum][rownum][0],block_row_size[compnum]);
                    modified++;
                }
                totalrows++;
            }
        }
        // Write to the output file
        jpeg_write_coefficients(&outputinfo, coef_arrays);
        // Finish compression and release memory
        jpeg_finish_compress(&outputinfo);
        jpeg_destroy_compress(&outputinfo);
        fclose(output_file);
    }
    jpeg_finish_decompress(&inputinfo);
    jpeg_destroy_decompress(&inputinfo);
    fclose(input_file);
    if ( modified != 0 )
    {
        printf("New DCT coefficients successfully written to %s, capacity %d modifiedrows.%d/%d emit.%d\n",outputfname,*capacityp,modified,totalrows,emit);
    }
    free(block_row_size);
    free(width_in_blocks);
    free(height_in_blocks);
    if ( data != origdata )
        free(data);
    return(modified);
}

char *LP_jpg(char *srcfile,char *destfile,int32_t power2,char *passphrase,char *datastr,int32_t required,uint16_t ind)
{
    cJSON *retjson; int32_t len=0,modified,capacity; char *decodedstr; uint8_t *data=0,*decoded=0;
    if ( srcfile != 0 && srcfile[0] != 0 )
    {
        retjson = cJSON_CreateObject();
        if ( datastr != 0 && datastr[0] != 0 )
        {
            if ( (len= is_hexstr(datastr,0)) > 0 )
            {
                len >>= 1;
                data = calloc(1,len);
                decode_hex(data,len,datastr);
                required = len * 8;
                //int32_t i; for (i=0; i<required; i++)
                //    printf("%c",'0'+(GETBIT(data,i)!=0));
                //printf(" datastr.%d %s\n",required,datastr);
            }
        }
        if ( required > 0 )
            decoded = calloc(1,len+required);
        if ( (modified= LP_jpg_process(&capacity,srcfile,destfile,decoded,data,required,power2,passphrase,&ind)) < 0 )
            jaddstr(retjson,"error","file not found");
        else
        {
            jaddstr(retjson,"result","success");
            jaddnum(retjson,"modifiedrows",modified);
            if ( modified != 0 )
                jaddstr(retjson,"outputfile",destfile);
            jaddnum(retjson,"power2",power2);
            jaddnum(retjson,"capacity",capacity);
            jaddnum(retjson,"required",required);
            jaddnum(retjson,"ind",ind);
        }
        if ( decoded != 0 )
        {
            if ( capacity > 0 )
            {
                //printf("len.%d required.%d capacity.%d\n",len,required,capacity);
                decodedstr = calloc(1,(len+required)*2+1);
                init_hexbytes_noT(decodedstr,decoded,required/8);
                jaddstr(retjson,"decoded",decodedstr);
                free(decodedstr);
            }
            free(decoded);
        }
        if ( data != 0 )
            free(data);
        return(jprint(retjson,1));
    } else return(clonestr("{\"error\":\"no source file error\"}"));
}




