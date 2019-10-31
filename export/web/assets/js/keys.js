var private_prefix = '55';
var public_prefix = '19';
var key = null;
var my_tx = null;
var paytxfee = 10000;
var nSignedInput = 0;

var anon_access = false;
var stake_infos = {};
var MyAccount = null;


if (!Uint8Array.prototype.slice && 'subarray' in Uint8Array.prototype)
    Uint8Array.prototype.slice = Uint8Array.prototype.subarray;

function pk2addr(pubkey)
{
    var faddr, paddr, eaddr;
    var h, h2;
    addr = public_prefix + RMDstring(hex2a(sha256(pubkey)));

    h = sha256(addr);
    h2 = sha256(h);
    crc = h2.slice(0, 8);
    faddr = addr + crc;
    paddr = hex2b(faddr);
    return to_b58(paddr);

}


function pubkey_to_addr(pubkey) {

    $('#pubaddr').val(pk2addr(pubkey));

    /*
    console.log("faddr : " + faddr + " paddr " + paddr + " eaddr " + eaddr);

    rpc_call('pubkeytoaddr', [pubkey], function (data) {
        console.log("addr : " + eaddr + " result " + data.result.addr);
        $('#pubaddr').val(data.result.addr);
    });
    */
}


function privKeyAddr(username, addr, secret) {
    var acName = username.replace('@', '-');

    if (acName == "anonymous") {
        anon_rpc_call('dumpprivkey', [addr], function (keyData) {
            $('#privAddr').html(keyData.result.privkey);
        });
    }
    else {
        rpc_call('getprivaddr', [acName, addr], function (keyData) {
            var faddr, paddr, eaddr, crc;
            var xk = keyData.result.privkey.slice(0, 64);
            var DecHexkey = strtoHexString(un_enc(secret, xk));
            var addr = private_prefix + DecHexkey + '01';

            h = sha256(addr);
            h2 = sha256(h);
            crc = h2.slice(0, 8);
            faddr = addr + crc;
            paddr = hex2b(faddr);
            eaddr = to_b58(paddr);
            $('#privAddr').html(eaddr);
        });
    }
}


function newkey() {
    var addr, sk, hexk;
    addr = $('#privkey').val();

    if (addr.length == 64)
        hexk = addr;
    else
    {
        data = from_b58(addr, ALPHABET);
        if (!data)
        {
            console.log('unable to decode key');
            return;
        }
        
        crc = toHexString(data.slice(34, 38));
        sk = data.slice(0, 34);
        hexk = toHexString(sk);
        h = sha256(hexk);
        h2 = sha256(h);
        if (crc != h2.slice(0, 8))
            alert('bad key');

        sk = data.slice(1, 33);
        hexk = toHexString(sk);
    }
    
    key = ec.keyPair({ priv: hexk, privEnc: 'hex' });
    if (key == null)
    {
        console.log('error key');
        return;
    }
    pubkey = key.getPublic().encodeCompressed('hex');
    privkey = hexk;
    $('#pubaddr').val(pk2addr(pubkey));

    
}


function check_key(privKey, pubAddr) {
    var test_key = ec.keyPair({ priv: privKey, privEnc: 'hex' });
    pubkey = test_key.getPublic().encodeCompressed('hex');

    var addr = pk2addr(pubkey);

    //rpc_call('pubkeytoaddr', [pubkey], function (data) {
    if (addr != pubAddr) {
        $('#selected_' + pubAddr).prop('checked', false);
        $('#secret_' + pubAddr).css('color', 'red');
    }
    else {
        $('#selected_' + pubAddr).prop('checked', true);
        $('#secret_' + pubAddr).css('color', 'green');
    }
    //});
}

function rc4_cypher(key, str) {
    var s = [], j = 0, x, res = '';
    for (var i = 0; i < 256; i++) {
        s[i] = i;
    }
    for (i = 0; i < 256; i++) {
        j = (j + s[i] + key.charCodeAt(i % key.length)) % 256;
        x = s[i];
        s[i] = s[j];
        s[j] = x;
    }
    i = 0;
    j = 0;
    for (var y = 0; y < str.length; y++) {
        i = (i + 1) % 256;
        j = (j + s[i]) % 256;
        x = s[i];
        s[i] = s[j];
        s[j] = x;
        res += String.fromCharCode(str.charCodeAt(y) ^ s[(s[i] + s[j]) % 256]);
    }
    return res;
}


function rc4_cypher_arr(key, arr) {
    var s = [], j = 0, x, res = '';
    for (var i = 0; i < 256; i++) {
        s[i] = i;
    }
    for (i = 0; i < 256; i++) {
        j = (j + s[i] + key.charCodeAt(i % key.length)) % 256;
        x = s[i];
        s[i] = s[j];
        s[j] = x;
    }
    i = 0;
    j = 0;
    for (var y = 0; y < arr.length; y++) {
        i = (i + 1) % 256;
        j = (j + s[i]) % 256;
        x = s[i];
        s[i] = s[j];
        s[j] = x;
        res += String.fromCharCode(arr[y] ^ s[(s[i] + s[j]) % 256]);
    }
    return res;
}

function rc4_cypher_ak(key, str) {
    var s = [], j = 0, x, res = [];

    for (var i = 0; i < 256; i++) {
        s[i] = i;
    }
    for (i = 0; i < 256; i++) {
        j = (j + s[i] + key[i % key.length]) % 256;
        x = s[i];
        s[i] = s[j];
        s[j] = x;
    }
    i = 0;
    j = 0;
    for (var y = 0; y < str.length; y++) {
        i = (i + 1) % 256;
        j = (j + s[i]) % 256;
        x = s[i];
        s[i] = s[j];
        s[j] = x;
        res.push (str.charCodeAt(y) ^ s[(s[i] + s[j]) % 256]);
    }
    return res;
}

function rc4_cypher_arr_ak(key, arr) {
    var s = [], j = 0, x, res = '';
    for (var i = 0; i < 256; i++) {
        s[i] = i;
    }
    for (i = 0; i < 256; i++) {
        j = (j + s[i] + key[i % key.length]) % 256;
        x = s[i];
        s[i] = s[j];
        s[j] = x;
    }
    i = 0;
    j = 0;
    for (var y = 0; y < arr.length; y++) {
        i = (i + 1) % 256;
        j = (j + s[i]) % 256;
        x = s[i];
        s[i] = s[j];
        s[j] = x;
        res += String.fromCharCode(arr[y] ^ s[(s[i] + s[j]) % 256]);
    }
    return res;
}


function un_enc(secret, HexKey) {
    var strKey;
    strKey = hex2a(HexKey);

    return rc4_cypher(secret, strKey);
}


function check_ecdsa() {
    // Generate keys
    var key = ec.genKeyPair();

    // Sign message (must be an array, or it'll be treated as a hex sequence)
    var msg = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
    var signature = key.sign(msg);
    // Export DER encoded signature in Array
    var derSign = signature.toDER();

    // Verify signature
    console.log(key.verify(msg, derSign));
}

function generateKeys() {
    var ec = new EC('secp256k1');
    // Generate keys
    key = ec.genKeyPair();
    $('#privkey').val(key.getPrivate('hex'));

    pubkey = key.getPublic().encodeCompressed('hex');
    privkey = key.getPrivate('hex');
    $('#pubaddr').val(pk2addr(pubkey));
}

function AccountList(divName,listName,opts) {

    var n;
    var panel,input, span, div, inner, p,a,container, row, col1, col2, label, table, h2;
    var self = this;

    this.opts = opts;
    this.accountSelects = new Array();
    this.accountName = '';
    this.staking_unspents = null;
    this.unspents = null;
    this.minConf = 10;
    this.selectedMenu = '';
    this.SelectedAddrs = new Array();
    this.staketimer = null;
    this.stakeUpdateTimer = null;
    this.nHashes = 0;
    this.TxTable = null;
    this.AddrTable = null;
    this.AnonTimeout = null;
    this.n_parsed_tx = 0;
    div = document.getElementById(divName);
    container = document.createElement('div');

    container.className = "card";

    $('#anon_timeout').focusin(function () {
    
        /* console.log(' focus :' + self.AnonTimeout); */
        
        if (self.AnonTimeout != null)
        {
            clearTimeout(self.AnonTimeout);
            self.AnonTimeout = null;
        }
    });

    $('#anon_timeout').focusout(function () {

        /* console.log(' unfocus :' + self.AnonTimeout); */
        self.AnonTimeout = setTimeout(function () { self.update_timeout(); }, 1000);
    });
    
        
    /* header */        
    row = document.createElement('div');
    row.className = "card-header  pt-3 aqua-gradient";
    h2= document.createElement('h3');
    h2.className = 'white-text mb-3 pt-3 font-weight-bold';
    h2.innerHTML = 'Manage your wallet';
    row.appendChild(h2);
    container.appendChild(row);

    /* body */
    row = document.createElement('div');
    row.className = "card-body px-lg-5 pt-0";

    /* account select */
    h2 = document.createElement('h3');
    h2.innerHTML = 'Select an account';

    col1 = document.createElement('div');
    col1.className = "md-form";

    this.select = document.createElement('select');
    this.select.id = "my_account";
    this.select.name = "my_account";
    this.select.className = "browser-default";

    col1.appendChild(h2);
    col1.appendChild(this.select);
    row.appendChild(col1);

    panel = document.createElement('div');
    panel.id = "account_addresses_loading";
    row.appendChild(panel);


    panel = document.createElement('div');
    panel.id = "account_addresses";

    /* account name */
    col1 = document.createElement('div');
    col1.className = 'md-form';

    label = document.createElement('label');
    label.id = 'account-label';
    label.setAttribute('for', 'account_name');
    label.innerHTML = 'Enter the name of the new account and press add new address button.';

    this.input = document.createElement('input');
    this.input.id = "account_name";
    this.input.type = 'text';
    this.input.className = 'form-control';  
        
    col1.appendChild(this.input);
    col1.appendChild(label);
    panel.appendChild(col1);

    col1 = document.createElement('div');
    col1.className = 'md-form';

    a = document.createElement('a');
    a.id='newKeyBut';
    a.className = "btn btn-primary waves-effect waves-light"
    a.setAttribute('data-toggle', 'modal');
    a.setAttribute('data-target', '#newKeyModal');
    a.setAttribute('data-backdrop', 'false');
  

    if (!this.opts.newAccnt)
        a.style.display = 'none';
    else
        a.addEventListener("click", function () { var accountName = self.input.value; if (accountName.length == 0) { $('#import_error').html('Enter or select an account name on the main page'); $('#import_btn').prop('disabled', true); } else { $('#import_error').html(''); $('#import_btn').prop('disabled', false); } });

    a.innerHTML = 'add new address';
    col1.appendChild(a);
    panel.appendChild(col1);

    /* rescan all */
    col1 = document.createElement('div');
    col1.id = 'account_infos';
    col1.style.display = 'none';

    inner = document.createElement('div');
    inner.className = 'md-form';
    inner.style='height:32px;'

    input = document.createElement('input');
    input.id = "show_null";
    input.name = "show_null";
    input.type = "checkbox";
    input.value = '1';
    input.className = 'custom-control-input';

    if (this.opts.shownull)
    {
        input.setAttribute('checked', 'checked');
    }

    input.onchange = function () { self.accountselected(self.accountName); }

    label = document.createElement('label');
    label.className = 'custom-control-label';
    label.style = 'margin:12px;'
    label.setAttribute('for', 'show_null');
    label.innerHTML = 'show address with no balance';

    inner.appendChild(input);
    inner.appendChild(label);
    col1.appendChild(inner);

    inner = document.createElement('div');
    inner.className = 'md-form';

    input = document.createElement('button');
    input.className = 'btn btn btn-primary';
    input.type = "button";
    input.onclick = function () { self.scan_account(); }
    input.innerHTML = 'rescan all';

    inner.appendChild(input);
    col1.appendChild(inner);

    inner = document.createElement('div');
    inner.className = 'row';
    col2 = document.createElement('div');
    col2.className = 'col-md-2';
    col2.innerHTML = 'total:';
    inner.appendChild(col2);
    col2 = document.createElement('div');
    col2.className = 'col-md-4 justify-content-end';
    col2.id = 'addr-total';
    inner.appendChild(col2);
    col1.appendChild(inner);

    inner = document.createElement('div');
    inner.className = 'row';
    col2 = document.createElement('div');
    col2.className = 'col-md-2';
    col2.innerHTML = 'selected:';
    inner.appendChild(col2);

    col2 = document.createElement('div');
    col2.className = 'col-md-4 justify-content-end';
    col2.id = 'addr-selected';
    inner.appendChild(col2);
    col1.appendChild(inner);

    panel.appendChild(col1);

    /* address list */
    col1 = document.createElement('div');
    

    this.p = document.createElement('p');
    this.p.style.display = 'none';
    this.p.innerHTML = 'Below are the addresses for your account.'

    col1.appendChild(this.p);

    this.create_addr_table();

    col1.appendChild(this.AddrTable);

     
    panel.appendChild(col1);
    row.appendChild(panel);

    container.appendChild(row);
    div.appendChild(container);

    row = this.makeModal('newKeyModal');
    row.appendChild(this.newAddrForm());

    a = document.createElement('a');
    a.type = 'button';
    a.className = 'btn btn-outline-info waves-effect';
    a.setAttribute('data-dismiss', 'modal');
    a.innerHTML = 'close';
    row.appendChild(a);

    container = document.createElement('div');
    div.appendChild(container);

    row = this.makeModal('PrivateKeyModal');
    p = document.createElement('p');
    p.id = 'unspentaddr';
    row.appendChild(p);


    inner = document.createElement('div');
    inner.className = 'md-form';

    label = document.createElement('label');
    label.setAttribute('for', 'viewPrivSecret');
    label.innerHTML = 'enter your secret :';

    input = document.createElement('input');
    input.id = "viewPrivSecret";
    input.name = "viewPrivSecret";
    input.type = "password";
    input.className = 'form-control';

    inner.appendChild(input);
    inner.appendChild(label);
    row.appendChild(inner);

    inner = document.createElement('div');
    inner.className = 'text-center';
    p = document.createElement('span');
    p.id = 'privAddr';
    inner.appendChild(p);
    row.appendChild(inner);

    a = document.createElement('a');
    a.type = "button";
    a.innerHTML = 'get private addr';
    a.className = 'btn btn-info waves-effect waves-light';
    a.onclick = function () { privKeyAddr(MyAccount.accountName, $('#unspentaddr').html(), $('#viewPrivSecret').val()); }

    row.appendChild(a);

    a = document.createElement('a');
    a.type = 'button';
    a.className = 'btn btn-outline-info waves-effect';
    a.setAttribute('data-dismiss', 'modal');
    a.innerHTML = 'close';
    row.appendChild(a);

    // build transaction list
    if ((listName != null) && (listName.length > 0)) {
        var cbody, ul, li;
        
        div = document.getElementById(listName);
        container = document.createElement('div');
        row = document.createElement('div');

        container.className = "card text-center";
        row.className = "card-header";

        h2 = document.createElement('div');
        h2.className = 'white-text mb-3 pt-3 font-weight-bold';
        h2.innerHTML = 'Manage your transactions';
        row.appendChild(h2);

        ul = document.createElement('ul');
        ul.className = 'nav md-pills nav-justified pills-primary';

        li = document.createElement('li');
        li.className = 'nav-item';

        a = document.createElement('a');
        a.className = 'nav-link';
        a.id = 'tab_unspents';
        a.innerHTML = 'unspent';
        a.addEventListener("click", function () { self.fetch_unspents(); });

        li.appendChild(a);
        ul.appendChild(li);

        li = document.createElement('li');
        li.className = 'nav-item';

        a = document.createElement('a');
        a.className = 'nav-link';
        a.id = 'tab_spents';
        a.innerHTML = 'spent';
        a.addEventListener("click", function () { self.fetch_spents(); });

        li.appendChild(a);
        ul.appendChild(li);

        li = document.createElement('li');
        li.className = 'nav-item';

        a = document.createElement('a');
        a.className = 'nav-link';
        a.id = 'tab_received';
        a.innerHTML = 'received';
        a.addEventListener("click", function () { self.fetch_recvs(); });

        li.appendChild(a);
        ul.appendChild(li);
        row.appendChild(ul);
        container.appendChild(row);


        cbody = document.createElement('div');
        cbody.className = "card-body";

        row = document.createElement('div');
        row.className = 'row';
        col1 = document.createElement('div');
        col1.className = 'col-sm-1';
        col1.innerHTML = 'total:';
        row.appendChild(col1);
        col1 = document.createElement('div');
        col1.id = 'txtotal';
        col1.className = 'col-md-2 justify-content-end';
        row.appendChild(col1);
        cbody.appendChild(row);

        row = document.createElement('div');
        row.className = 'row';
        col1 = document.createElement('div');
        col1.className = 'col-sm-1';
        col1.innerHTML = 'selected:';
        row.appendChild(col1);
        col1 = document.createElement('div');
        col1.id = 'selected_balance';
        col1.className = 'col-md-2 justify-content-end';
        row.appendChild(col1);
        cbody.appendChild(row);

        row = document.createElement('div');
        row.className = 'row';
        col1 = document.createElement('div');
        col1.className = 'col-sm-1';
        col1.innerHTML = 'showing:';
        row.appendChild(col1);
        col1 = document.createElement('div');
        col1.className = 'col-md-2 justify-content-end';
        col1.innerHTML = '<span id="ntx"></span>/<span id="total_tx"></span>';
        row.appendChild(col1);
        cbody.appendChild(row);
        col1 = document.createElement('div');

        this.create_tx_table();

        col1.appendChild(this.TxTable);
        cbody.appendChild(col1);
        container.appendChild(cbody);
        div.appendChild(container);
    }
}

AccountList.prototype.seteventsrc = function (in_url, handler, handler2) {
    var self = this;

    this.evtSource = new EventSource(site_base_url + in_url);

    this.evtSource.addEventListener("newspent", handler, false);

    if(handler2)
        this.evtSource.addEventListener("newunspent", handler2, false);

    

}

AccountList.prototype.create_addr_table = function () {

    var ths = ["label", "balance", "unconf"];
    var newTable;

    if (this.opts.withSecret)
        ths.push("secret");

    ths.push("select");
    ths.push("rescan");

    newTable = document.createElement('table');
    newTable.id = "my_address_list_table";
    newTable.className = "table table-hover";

    var header = newTable.createTHead();
    var trow = header.insertRow(0);

    header.className = "black white-text";

    for (var n = 0; n < ths.length; n++) {
        var th = document.createElement('th');

        if (ths[n] == 'unconf') th.className = 'balance_unconfirmed';
        if (ths[n] == 'rescan') th.className = 'scan';
        th.innerHTML = ths[n];
        trow.appendChild(th);
    }

    newTable.appendChild(document.createElement('tbody'));

    if (this.AddrTable != null)
        this.AddrTable.parentNode.replaceChild(newTable, this.AddrTable);

    this.AddrTable = newTable;
}

AccountList.prototype.create_tx_table = function () {

    var ths = ["time", "tx", "from", "amount", "nconf"];
    var newTable;

    newTable = document.createElement('table');
    newTable.id = 'tx_list';
    newTable.className = "table hover";

    var header = newTable.createTHead();
    var trow = header.insertRow(0);

    for (var n = 0; n < ths.length; n++) {
        var th = document.createElement('th');

        if (ths[n] == 'tx') th.className = 'tx-cell';
        if (ths[n] == 'time') th.className = 'time';
        th.innerHTML = ths[n];
        trow.appendChild(th);
    }

    newTable.appendChild(document.createElement('tbody'));

    if (this.TxTable != null)
        this.TxTable.parentNode.replaceChild(newTable, this.TxTable);

    this.TxTable = newTable;
}

AccountList.prototype.addr_secret_change = function ()
{
    var addr = this.getAttribute('address');

    $('#selected_' + addr).prop  ('checked',false); 
    self.update_unspent          ();
}

AccountList.prototype.select_menu = function   (id)
{
    if (this.selectedMenu == id) return;

    this.selectedMenu = id;

    if (id == "tab_unspents")
        $('#tab_unspents').addClass('active');
    else
        $('#tab_unspents').removeClass('active');

    if (id == "tab_spents")
        $('#tab_spents').addClass('active');
    else
        $('#tab_spents').removeClass('active');

    if (id == "tab_received")
        $('#tab_received').addClass('active');
    else
        $('#tab_received').removeClass('active');
}

AccountList.prototype.staking_loop = function   (hash_data, time_start, time_end, diff) {
    var ct;
    for (ct = time_start; ct < time_end; ct += 16) {
        var str = hex32(ct);
        var total = hash_data + str;
        var h = sha256(total);
        var h2 = sha256(h);
        if (compare_hash(h2, diff)) {
            $('#newhash').html(h2);
            return ct;
        }
        this.nHashes++;
    }
    return 0;
}

AccountList.prototype.check_all_staking = function () {
    var self = this;

    if (!$('#do_staking').prop('checked')) return 0;

    if (this.staking_unspents == null) return 0;

    var n;
    var time_start, time_end;
    var timeStart = Math.floor(new Date().getTime() / 1000);
    var timeBegin = Math.floor((timeStart + 15) / 16) * 16;

    var num_stake_unspents = this.staking_unspents.length;

    if (stake_infos.last_block_time > (stake_infos.now - stake_infos.block_target)) {
        time_start = Math.floor((stake_infos.last_block_time + 15) / 16) * 16;
        time_end = time_start + stake_infos.block_target;
    }
    else {
        time_start = timeBegin - 16;
        time_end = timeBegin + 16;
    }
    this.nHashes = 0;

    for (n = 0; n < num_stake_unspents; n++) {
        var txtime;

        if (this.staking_unspents[n].inactive)
            continue;

        if (!this.staking_unspents[n].hash_data)
            continue;

        var staking = this.staking_unspents[n];



        txtime = this.staking_loop(staking.hash_data, time_start, time_end, staking.difficulty);

        var timeEnd = Math.ceil(new Date().getTime() / 1000);
        var timespan = (timeEnd - timeStart);
        var hashrate = this.nHashes / timespan;

        $('#hashrate').html(this.nHashes + ' hashes in ' + timespan + ' secs (' + hashrate + ' hashes/sec) last scan : ' + timeStart);

        if (txtime > 0) {
            var pubkey = $('#selected_' + staking.dstaddr).attr('pubkey');
            rpc_call('getstaketx', [staking.txid, staking.vout, txtime, pubkey], function (staketx) {
                var txh, txa, secret;

                if (staketx.error) {
                    self.fetch_unspents();
                    console.log('stake tx rejected');
                    return;
                }

                var bh = staketx.result.newblockhash;
                txh = staketx.result.txhash;
                txa = staketx.result.addr;

                rpc_call('getprivaddr', [self.accountName, txa], function (keyData) {

                    if (keyData.error) {
                        console.log('stake tx rejected');
                        return;
                    }
                    secret = $('#secret_' + txa).val();
                    var DecHexkey = strtoHexString(un_enc(secret, keyData.result.privkey.slice(0, 64)));
                    var keys = ec.keyPair({ priv: DecHexkey, privEnc: 'hex' });

                    // Sign message (must be an array, or it'll be treated as a hex sequence)
                    var pubk = keys.getPublic().encodeCompressed('hex');
                    var signature = keys.sign(txh, 'hex');

                    // Export DER encoded signature in Array
                    //var derSign = signature.toDER('hex');
                    var derSign = signature.toLowS();

                    rpc_call('signstaketx', [bh, derSign, pubk], function (txsign) {
                        var hash = txsign.result.newblockhash;
                        var blocksignature = keys.sign(hash, 'hex');
                        var derSign = blocksignature.toLowS();

                        rpc_call('signstakeblock', [hash, derSign, pubk], function (blksign) {
                            self.fetch_unspents();
                        });
                    });
                });
            });
            return;
        }
    }

  

    /* self.fetch_unspents(); */
    /*
    if (this.staketimer != null)
        clearTimeout(this.staketimer);

    this.staketimer = setTimeout(function () { self.check_all_staking(); }, 10000);
    */
}

AccountList.prototype.update_objects = function () {
    var total;
    var n, nrows;
    var thead;
    var num_objects;

    this.create_tx_table();

    this.selectedObjects = new Array();

    if (this.objects == null) {
        return;
    }

    num_objects = this.objects.length;
    thead = this.TxTable.tHead;
    thead.rows[0].cells[2].innerHTML = 'from';

    total = 0;
    nrows = 0;


    for (n = 0; n < num_objects; n++) {

        if (!$('#selected_' + this.objects[n].dstaddr).is(':checked')) continue;
        var cell;
        var naddr;
        var addresses;
        var row = this.TxTable.tBodies[0].insertRow(nrows++);

        if (this.objects[n].confirmations >= this.minConf) {
            this.selectedObjects.push(this.objects[n]);
            row.className = 'tx_ready';
        }
        else
            row.className = 'tx_unconf';


        cell = row.insertCell(0);
        cell.className = "app";
        cell.innerHTML = this.objects[n].appName;

        cell = row.insertCell(1);
        cell.className = "type";
        cell.innerHTML = this.objects[n].appType;

        cell = row.insertCell(2);
        cell.className = "tx-cell";
        cell.innerHTML = this.objects[n].objHash;

        cell = row.insertCell(3);
        cell.className = "time";
        cell.innerHTML = timeConverter(this.objects[n].time);

        cell = row.insertCell(4);
        cell.className = "tx-cell";
        cell.innerHTML = this.objects[n].txid;


        cell = row.insertCell(5);
        cell.className = "addr-cell";
        naddr = this.objects[n].addresses.length;
        addresses = '';

        while (naddr--) {
            addresses += this.objects[n].addresses[naddr] + '<br/>';
        }

        cell.innerHTML = addresses;

        cell = row.insertCell(6);
        cell.className = "tx_conf";
        cell.innerHTML = this.objects[n].confirmations;

    }

    for (n = 0; n < num_objects; n++) {

        if ($('#selected_' + this.objects[n].dstaddr).is(':checked')) continue;
        var cell;
        var naddr;
        var addresses;
        var row = this.TxTable.tBodies[0].insertRow(nrows++);

        row.className = 'tx_error';

        cell = row.insertCell(0);
        cell.className = "app";
        cell.innerHTML = this.objects[n].appName;

        cell = row.insertCell(1);
        cell.className = "type";
        cell.innerHTML = this.objects[n].appType;

        cell = row.insertCell(2);
        cell.className = "tx-cell";
        cell.innerHTML = this.objects[n].objHash;

        cell = row.insertCell(3);
        cell.className = "time";
        cell.innerHTML = timeConverter(this.objects[n].time);

        cell = row.insertCell(4);
        cell.className = "tx-cell";
        cell.innerHTML = this.objects[n].txid;

        cell = row.insertCell(5);
        cell.className = "addr-cell";
        naddr = this.objects[n].addresses.length;
        addresses = '';

        while (naddr--) {
            addresses += this.unspents[n].addresses[naddr] + '<br/>';
        }

        cell.innerHTML = addresses;

        cell = row.insertCell(6);
        cell.className = "tx_conf";
        cell.innerHTML = this.objects[n].confirmations;
    }

    $('#txtotal').html(0);
    $('#selected_balance').html(0);

}

AccountList.prototype.update_unspent = function () {
    var total;
    var n,nrows;
    var thead;
    var num_unspents;
    
    this.create_tx_table         ();

    this.selectedUnspents = new Array();

    if (this.unspents == null) {
        $('#txtotal').html(0);
        $('#selected_balance').html(0);
        return;
    }

    num_unspents = this.unspents.length;
    thead = this.TxTable.tHead;
    thead.rows[0].cells[2].innerHTML = 'from';
                
    this.selected_balance               = 0;
    total                               = 0;
    nrows                               = 0;


    for (n = 0; n < num_unspents; n++) {

        if (!$('#selected_' + this.unspents[n].dstaddr).is(':checked')) continue;
        var cell;
        var naddr;
        var addresses;
        var row = this.TxTable.tBodies[0].insertRow(nrows++);
        
        if (this.unspents[n].confirmations >= this.minConf) {
            this.selectedUnspents.push(this.unspents[n]);
            this.selected_balance += this.unspents[n].amount;
            row.className = 'tx_ready';
        }
        else
            row.className = 'tx_unconf';
                  

        cell            = row.insertCell(0);
        cell.className  = "time";
        cell.innerHTML  = timeConverter(this.unspents[n].time);

        cell            = row.insertCell(1);
        cell.className  = "tx-cell";
        cell.innerHTML  = this.unspents[n].txid;


        cell            = row.insertCell(2);
        cell.className = "addr-cell";

        addresses = '';

        if (typeof this.unspents[n].addresses != 'undefined') {

            naddr = this.unspents[n].addresses.length;


            while (naddr--) {
                addresses += this.unspents[n].addresses[naddr] + '<br/>';
            }
        }
        else
            addresses = this.unspents[n].dstaddr;

          cell.innerHTML = addresses;

        cell = row.insertCell(3);
        cell.className = "addr_amount";
        cell.innerHTML = this.unspents[n].amount / unit;

        cell = row.insertCell(4);
        cell.className = "tx_conf";
        cell.innerHTML = this.unspents[n].confirmations;

        total += this.unspents[n].amount;
    }

    for (n = 0; n < num_unspents; n++) {

        if ($('#selected_' + this.unspents[n].dstaddr).is(':checked')) continue;
        var cell;
        var naddr;
        var addresses;
        var row = this.TxTable.tBodies[0].insertRow(nrows++);

        row.className   = 'tx_error';

        cell            = row.insertCell(0);
        cell.className  = "time";
        cell.innerHTML  = timeConverter(this.unspents[n].time);

        cell            = row.insertCell(1);
        cell.className = "tx-cell";
        cell.innerHTML  = this.unspents[n].txid;

        cell            = row.insertCell(2);
        cell.className = "addr-cell";

        addresses = '';

        if (typeof this.unspents[n].addresses != 'undefined') {

            naddr = this.unspents[n].addresses.length;
          

            while (naddr--) {
                addresses += this.unspents[n].addresses[naddr] + '<br/>';
            }
        }
        else
            addresses = this.unspents[n].dstaddr;

        cell.innerHTML  = addresses;

        cell            = row.insertCell(3);
        cell.className  = "addr_amount";
        cell.innerHTML  = this.unspents[n].amount / unit;

        cell            = row.insertCell(4);
        cell.className  = "tx_conf";
        cell.innerHTML  = this.unspents[n].confirmations;

        total          += this.unspents[n].amount;
    }
    
    $('#txtotal').html                  (total / unit);
    $('#selected_balance').html         (this.selected_balance / unit);

}



AccountList.prototype.update_staking_infos = function ()
{
    if ((this.staking_unspents!=null)&&(this.staking_unspents.length > 0)) {
        $('#do_staking').prop('disabled', false);

        this.totalweight = 0;
        this.nActive = 0;
        for (var n = 0; n < this.staking_unspents.length; n++) {

            if (!this.staking_unspents[n].inactive)
            {
                this.totalweight += this.staking_unspents[n].weight;
                this.nActive++;
            }
                
        }
        $('#stakeweight').html(this.totalweight / unit);
        $('#nstaketxs').html(this.nActive);
        $('#stake_msg').empty();
    }
    else {
        $('#do_staking').prop('disabled', true);
        $('#stakeweight').html('0');
        $('#nstaketxs').html('0');
        $('#stake_msg').html('no suitable unspent found');
    }
}

AccountList.prototype.fetch_staking_unspents = function  () {
    var self = this;
    var n;

    if (this.addrs == null) return;

    if (this.SelectedAddrs.length == 0)
    {
        this.staking_unspents = null;
        this.update_staking_infos();
        return;
    }

    if (this.staketimer != null)
        clearTimeout(this.staketimer);

    rpc_call('liststaking', [0, 9999999, this.SelectedAddrs],

    function (data) {
        var     n;

        self.staking_unspents               = data.result.unspents;
        stake_infos.block_target            = data.result.block_target;
        stake_infos.now                     = data.result.now;
        stake_infos.last_block_time         = data.result.last_block_time;

        self.update_staking_infos();

        if (self.staketimer != null)
            clearTimeout(self.staketimer);

        self.staketimer = setTimeout(function () { self.check_all_staking(); }, 10000);

        if (self.stakeUpdateTimer != null)
            clearTimeout(self.stakeUpdateTimer);

        self.stakeUpdateTimer = setTimeout(function () { self.fetch_unspents(); }, 60000);

    },
    function ()
    {
        if (self.staketimer != null)
            clearTimeout(self.staketimer);

        self.staketimer = setTimeout(function () { self.check_all_staking(); }, 1000);

        if (self.stakeUpdateTimer != null)
            clearTimeout(self.stakeUpdateTimer);

        self.stakeUpdateTimer = setTimeout(function () { self.fetch_unspents(); }, 1000);
    });
}

AccountList.prototype.fetch_unspents = function () {
    if (this.TxTable == null)return;

    var n;
    var self      = this;
    var AddrAr = [];

    this.create_tx_table();
    if (this.addrs == null) return;

    for (n = 0; n < this.addrs.length; n++) {

        if (this.SelectedAddrs.indexOf(this.addrs[n].address)>=0)
            AddrAr.push(this.addrs[n].address);
    }

    for (n = 0; n < this.addrs.length; n++) {

        if (this.SelectedAddrs.indexOf(this.addrs[n].address) < 0)
            AddrAr.push(this.addrs[n].address);
    }

    this.select_menu("tab_unspents");

    rpc_call('listunspent', [0, 9999999, AddrAr], function (data) {

        self.unspents = data.result.unspents;
        self.unspents.sort(function (a, b) { return (b.time - a.time); });

        self.update_unspent();

        $('#txtotal').html(data.result.total / unit);
        $('#ntx').html(self.unspents.length);
        $('#total_tx').html(data.result.ntx);

        if (self.opts.staking == true)
            self.fetch_staking_unspents();
    });

}

AccountList.prototype.transfer_objects = function (txh, oidx, dstAddr,fee) {

    return rpc_call_promise('makeobjtxfr', [txh, oidx, dstAddr,fee,0, 9999999], true);
}

AccountList.prototype.fetch_objects = function (appName,appType,addrs) {
    var n;
    var self = this;
    var AddrAr = [];
    
    if (addrs != null) {
        for (n = 0; n < addrs.length; n++) {
           AddrAr.push(addrs[n].address);
        }
    }
    else {

        if (this.addrs == null) return;

        for (n = 0; n < this.addrs.length; n++) {

            if (this.SelectedAddrs.indexOf(this.addrs[n].address) >= 0)
                AddrAr.push(this.addrs[n].address);
        }

        for (n = 0; n < this.addrs.length; n++) {

            if (this.SelectedAddrs.indexOf(this.addrs[n].address) < 0)
                AddrAr.push(this.addrs[n].address);
        }
    }

    this.select_menu("tab_objects");

    return rpc_call_promise('listobjects', [appName, appType, 0, 9999999, AddrAr], true);

}

AccountList.prototype.update_spent = function () {
    var total;
    var thead;
    var thead;
   
    this.create_tx_table();

    if (this.spents == null) {
        return;
    }

    var nrows = 0;
    var num_spents = this.spents.length;

    thead = this.TxTable.tHead;
    this.selected_balance = 0;

    thead.rows[0].cells[2].innerHTML = 'to';
    total = 0;

    for (var n = 0; n < num_spents; n++) {
        total += this.spents[n].amount;
        if (!$('#selected_' + this.spents[n].srcaddr).is(':checked')) continue;

        var cell;
        var naddr;
        var addresses;
        var row = this.TxTable.tBodies[0].insertRow(nrows++);

        row.className = 'tx_ready';

        cell = row.insertCell(0);

        cell.className = "time";
        cell.innerHTML = timeConverter(this.spents[n].time);

        cell = row.insertCell(1);
        cell.className = "tx-cell";
        cell.innerHTML = this.spents[n].txid;


        cell = row.insertCell(2);
        cell.className = "addr-cell addr_to";
        naddr = this.spents[n].addresses.length;
        addresses = '';

        while (naddr--) {
            addresses += this.spents[n].addresses[naddr] + '<br/>';
        }
        cell.innerHTML = addresses;

        cell = row.insertCell(3);
        cell.className = "addr_amount";
        cell.innerHTML = this.spents[n].amount / unit;

        cell = row.insertCell(4);
        cell.className = "tx_conf";
        cell.innerHTML = this.spents[n].confirmations;

        this.selected_balance += this.spents[n].amount;
        
    }

    for (var n = 0; n < num_spents; n++) {
        total += this.spents[n].amount;

        if ($('#selected_' + this.spents[n].srcaddr).is(':checked')) continue;
        var cell;
        var naddr;
        var addresses;
        var row = this.TxTable.tBodies[0].insertRow(nrows++);


        row.className   = 'tx_error';

        cell            = row.insertCell(0);
        cell.className  = "time";
        cell.innerHTML  = timeConverter(this.spents[n].time);

        cell            = row.insertCell(1);
        cell.className  = "tx-cell";
        cell.innerHTML  = this.spents[n].txid;

        cell            = row.insertCell(2);
        cell.className = "addr-cell addr_to";
        naddr           = this.spents[n].addresses.length;
        addresses       = '';

        while (naddr--) {
            addresses += this.spents[n].addresses[naddr] + '<br/>';
        }
        cell.innerHTML = addresses;

        cell = row.insertCell(3);
        cell.className = "addr_amount";
        cell.innerHTML = this.spents[n].amount / unit;

        cell = row.insertCell(4);
        cell.className = "tx_conf";
        cell.innerHTML = this.spents[n].confirmations;
    }

    $('#txtotal').html(total / unit);
    $('#selected_balance').html(this.selected_balance / unit);

    $('#ntx').html      (num_spents);
}

AccountList.prototype.fetch_spents = function () {
    var self      = this;
    var n;
    var AddrAr    = [];

    this.create_tx_table();

    if (this.addrs == null) return;

    for (n = 0; n < this.addrs.length; n++) {

        if (this.SelectedAddrs.indexOf(this.addrs[n].address) >= 0)
            AddrAr.push(this.addrs[n].address);
    }

    for (n = 0; n < this.addrs.length; n++) {

        if (this.SelectedAddrs.indexOf(this.addrs[n].address) < 0)
            AddrAr.push(this.addrs[n].address);
    }

    this.select_menu("tab_spents");

    rpc_call('listspent', [0, 9999999, AddrAr], function (data) {
      
        self.spents = data.result.spents;
        self.spents.sort(function (a, b) { return (b.time - a.time); });
        self.update_spent();

        $('#total_tx').html     (data.result.ntx);

    });
}


AccountList.prototype.fetch_sentobjs = function (appName, appType, addrs) {
    var n;
    var self = this;
    var AddrAr = [];

    if (addrs != null) {
        for (n = 0; n < addrs.length; n++) {
            AddrAr.push(addrs[n].address);
        }
    }
    else {

        if (this.addrs == null) return;

        for (n = 0; n < this.addrs.length; n++) {

            if (this.SelectedAddrs.indexOf(this.addrs[n].address) >= 0)
                AddrAr.push(this.addrs[n].address);
        }

        for (n = 0; n < this.addrs.length; n++) {

            if (this.SelectedAddrs.indexOf(this.addrs[n].address) < 0)
                AddrAr.push(this.addrs[n].address);
        }
    }

    this.select_menu("tab_sent-objects");

    return rpc_call_promise('listsentobjs', [appName, appType, 0, 9999999, AddrAr], true);

}

AccountList.prototype.update_recvs = function () {
    var thead;
    var nrows = 0;
    var total = 0;

    this.total_selected = 0;
    this.total_amount = 0;

    this.create_tx_table();

    if (this.recvs == null) 
        return;

    var num_recvs = this.recvs.length;

    thead = this.TxTable.tHead;
    thead.rows[0].cells[2].innerHTML = 'from';

    for (var n = 0; n < num_recvs; n++) {
        var cell;
        var naddr;
        var addresses;

        total += this.recvs[n].amount;
        if (!$('#selected_' + this.recvs[n].dstaddr).is(':checked')) continue;

        var row = this.TxTable.tBodies[0].insertRow(nrows++);
       
        row.className = 'tx_ready';
        cell = row.insertCell(0);

        cell.className = "time";
        cell.innerHTML = timeConverter(this.recvs[n].time);


        cell = row.insertCell(1);
        cell.className = "tx-cell";
        cell.innerHTML = this.recvs[n].txid;

        cell = row.insertCell(2);
        cell.className = "addr-cell addr_from";

        naddr = this.recvs[n].addresses.length;
        addresses = '';

        while (naddr--) {
            addresses += this.recvs[n].addresses[naddr] + '<br/>';
        }
        cell.innerHTML = addresses;

        cell = row.insertCell(3);
        cell.className = "addr_amount";
        cell.innerHTML = this.recvs[n].amount / unit;

        this.total_selected += this.recvs[n].amount;

        cell = row.insertCell(4);
        cell.className = "tx_conf";
        cell.innerHTML = this.recvs[n].confirmations;
    }

    for (var n = 0; n < num_recvs; n++) {
        var cell;
        var naddr;
        var addresses;

        total += this.recvs[n].amount;

        if ($('#selected_' + this.recvs[n].dstaddr).is(':checked')) continue;

        var row = this.TxTable.tBodies[0].insertRow(nrows++);
        

        row.className = 'tx_error';

        cell            = row.insertCell(0);
        cell.className  = "time";
        cell.innerHTML  = timeConverter(this.recvs[n].time);

        cell            = row.insertCell(1);
        cell.className  = "tx-cell";
        cell.innerHTML  = this.recvs[n].txid;

        cell            = row.insertCell(2);
        cell.className = "addr-cell addr_from";
        naddr           = this.recvs[n].addresses.length;
        addresses       = '';

        while (naddr--) {
            addresses += this.recvs[n].addresses[naddr] + '<br/>';
        }
        cell.innerHTML = addresses;

        cell = row.insertCell(3);
        cell.className = "addr_amount";
        cell.innerHTML = this.recvs[n].amount / unit;
        this.total_amount += this.recvs[n].amount;

        cell = row.insertCell(4);
        cell.className = "tx_conf";
        cell.innerHTML = this.recvs[n].confirmations;

       
    }

    $('#txtotal').html(total / unit);
    $('#selected_balance').html(this.total_selected / unit);
    $('#ntx').html      (num_recvs);
}

AccountList.prototype.fetch_recvs = function () {
    if (this.TxTable == null) return;
    var self      = this;
    var n;
    var AddrAr    = [];

    this.create_tx_table();

    if (this.addrs == null) return;

    for (n = 0; n < this.addrs.length; n++) {

        if (this.SelectedAddrs.indexOf(this.addrs[n].address) >= 0)
            AddrAr.push(this.addrs[n].address);
    }

    for (n = 0; n < this.addrs.length; n++) {

        if (this.SelectedAddrs.indexOf(this.addrs[n].address) < 0)
            AddrAr.push(this.addrs[n].address);
    }

    this.select_menu("tab_received");

    rpc_call('listreceived', [0, 9999999, AddrAr], function (data) {
      
        self.recvs      = data.result.received;
        self.recvs.sort(function (a, b) { return (b.time - a.time); });

        self.update_recvs();
        $('#total_tx').html(data.result.ntx);

        
    });
}



AccountList.prototype.maketx = function (amount, fee, dstAddr) {

    var self = this;
    $('#div_newtxid').css('display', 'none');

    if (this.accountName == 'anonymous') {

        anon_rpc_call('sendfrom', [this.accountName, dstAddr, amount], function (data) {

            $('#sendtx_but').prop("disabled", true);

            $('#total_tx').empty();
            $('#newtx').empty();

            if (data.error) {

            }
            else {
                $('#div_newtxid').css('display', 'block');
                $('#newtxid').html(data.result.txid);
            }
        });
    }
    else {

        if (my_tx != null)
        {
            rpc_call('canceltx', [my_tx.txid], function (data) {

                rpc_call('maketxfrom', [self.SelectedAddrs, amount, dstAddr, fee], function (data) {

                    my_tx = data.result.transaction;

                    $('#sendtx_but').prop("disabled", false);
                    $('#total_tx').html(data.result.total);
                    $('#newtx').html(get_tmp_tx_html(my_tx));
                });

            });
        }
        else
        {
            rpc_call('maketxfrom', [this.SelectedAddrs, amount, dstAddr, fee], function (data) {

                my_tx = data.result.transaction;

                $('#sendtx_but').prop("disabled", false);
                $('#total_tx').html(data.result.total);
                $('#newtx').html(get_tmp_tx_html(my_tx));
            });
        }
        
        
    }
}

    
AccountList.prototype.getSelectedAddrs = function () {
    var n;
    var AddrAr = [];

    for (n = 0; n < this.addrs.length; n++) {
    if ($('#selected_' + this.addrs[n].address).is(':checked'))
        AddrAr.push(this.addrs[n].address);
    }
    return AddrAr;
}

AccountList.prototype.update_txs = function ()
{
    if (this.selectedMenu == 'tab_spents')
        this.fetch_spents();
    else if (this.selectedMenu == 'tab_received')
        this.fetch_recvs();
    else
        this.fetch_unspents();
}

AccountList.prototype.update_selected_amount = function () {
    var select_amount;
    select_amount = 0;

    for (var n = 0; n < this.SelectedAddrs.length; n++) {
        select_amount += parseInt($('#balance_' + this.SelectedAddrs[n]).attr('amount'));
    }
    $('#addr-selected').html(select_amount / unit);
}

AccountList.prototype.check_address = function (addr)
{
    var n;

    if ((this.opts.withSecret == false) || (this.accountName == 'anonymous'))
    {
        this.SelectedAddrs = new Array();

        for (n = 0; n < this.addrs.length; n++) {
            if ($('#selected_' + this.addrs[n].address).is(':checked'))
                this.SelectedAddrs.push(this.addrs[n].address);
        }

        if (this.addr_selected)
            this.addr_selected(addr);

        this.update_txs();
        this.update_selected_amount();
    }
    else
    {
        var self    = this;
        var secret  = $('#secret_' + addr).val();

        rpc_call('getprivaddr', [this.accountName, addr], function (keyData) {
            var stakeAddrAr = [];
            var DecHexkey = strtoHexString(un_enc(secret, keyData.result.privkey.slice(0, 64)));
            var test_key = ec.keyPair({ priv: DecHexkey, privEnc: 'hex' });
            var tpubkey = test_key.getPublic().encodeCompressed('hex');

            var maddr = pk2addr(tpubkey);

            //rpc_call('pubkeytoaddr', [tpubkey], function (data) {

            if (maddr != addr) {
                $('#selected_' + addr).prop('checked', false);
                $('#selected_' + addr).attr('pubkey', '');
                $('#selected_' + addr).attr('privkey', '');
                $('#secret_' + addr).css('color', 'red');
            }
            else {
                $('#selected_' + addr).attr('pubkey', tpubkey);
                $('#selected_' + addr).attr('privkey', DecHexkey);
                $('#secret_' + addr).css('color', 'green');
            }

            self.SelectedAddrs = new Array();

            for (n = 0; n < self.addrs.length; n++) {
                if ($('#selected_' + self.addrs[n].address).is(':checked'))
                    self.SelectedAddrs.push(self.addrs[n].address);
            }

            if (self.addr_selected)
                self.addr_selected(addr);

            self.update_txs();

            self.update_selected_amount();
          });
        //});
    }
}

AccountList.prototype.update_addrs_select = function (select_name) {
    var opt;
    var select = document.getElementById(select_name);
    if (select == null) return;
    
    while (select.options.length) {
        select.remove(0);
    }
    
    if ((this.addrs == null) || (this.addrs.length == 0)) {
        opt     = document.createElement('option');
        opt.txt = 'no addr';
        select.add(opt);
        return;
    }

    for (var i = 0; i < this.addrs.length; i++) {
        opt         = document.createElement('option');
        opt.text    = this.addrs[i].label;
        opt.value   = this.addrs[i].address;
        select.add(opt);
    }
}

AccountList.prototype.update_addrs = function ()
{
    var self = this;
    var total_amount = 0;

    this.create_addr_table();

    if ((this.addrs == null) || (this.addrs.length == 0)) {
        this.AddrTable.style.display = 'none';
    }
    else {
        var n, num_addrs = this.addrs.length;

        for (n = 0; n < num_addrs; n++) {
            var cell, span, input,label;
            var row = this.AddrTable.tBodies[0].insertRow(n);
            var a;
            
            row.className = "my_wallet_addr";

            cell = row.insertCell(0);
            cell.setAttribute("title", this.addrs[n].address)

            a = document.createElement('a');
            a.setAttribute("addr", this.addrs[n].address);
            a.className = "btn btn-primary waves-effect waves-light addr-label"
            a.setAttribute('data-toggle', 'modal');
            a.setAttribute('data-target', '#PrivateKeyModal');
            a.setAttribute('data-backdrop', 'false');
            a.innerHTML = this.addrs[n].label;
            a.addEventListener("click", function () { self.addr_selected(this.getAttribute('addr')); });
            cell.appendChild(a);

            span = document.createElement('span');
            span.setAttribute("amount", this.addrs[n].amount);
            span.className = 'addr-amount';
            span.id = 'balance_' + this.addrs[n].address;
            span.innerHTML = this.addrs[n].amount / unit;

            cell = row.insertCell(1);
            cell.className = "balance_confirmed";
            cell.appendChild(span);

            span = document.createElement('span');
            span.className = 'addr-amount';
            span.innerHTML = this.addrs[n].unconf_amount / unit;

            cell = row.insertCell(2);
            cell.className = "balance_unconfirmed";
            span.id = 'balance_unconf_' + this.addrs[n].address;
            cell.appendChild(span);

            total_amount += this.addrs[n].amount;

            if ((this.opts.withSecret) && (this.accountName!='anonymous'))
            {
                input = document.createElement('input');
                input.setAttribute("address", this.addrs[n].address);
                input.className = 'addr-secret';
                input.type = "text";
                input.id = 'secret_' + this.addrs[n].address;
                input.value = '';

                span = document.createElement('div');
                span.appendChild (input);
            
                cell = row.insertCell(3);
                cell.appendChild  (span);

                label = document.createElement('label');
                label.className = 'custom-control-label';
                label.setAttribute('for', 'selected_' + this.addrs[n].address);
                label.innerHTML = '&nbsp;';

                input = document.createElement('input');
                input.id = 'selected_' + this.addrs[n].address;
                input.type = "checkbox";
                input.className = 'custom-control-input';
                input.value = 1;
                input.setAttribute("addr", this.addrs[n].address);

                input.addEventListener("change", function () { self.check_address( this.getAttribute('addr')); });

                span = document.createElement('div');
                span.className = 'custom-control custom-checkbox';
                span.appendChild(input);
                span.appendChild(label);

                cell = row.insertCell(4);
                cell.className = "select";
                cell.appendChild(span);
                
                cell = row.insertCell(5);
            }
            else
            {
                label = document.createElement('label');
                label.className = 'custom-control-label';
                label.setAttribute('for', 'selected_' + this.addrs[n].address);
                label.innerHTML = '&nbsp;';

                input = document.createElement('input');
                input.id = 'selected_' + this.addrs[n].address;
                input.type = "checkbox";
                input.className = 'custom-control-input';
                input.value = 1;
                input.setAttribute("addr", this.addrs[n].address);

                input.addEventListener("change", function () { self.check_address( this.getAttribute('addr')); });

                span = document.createElement('div');
                span.className = 'custom-control custom-checkbox';
                
                span.appendChild(input);
                span.appendChild(label);
                
                cell            = row.insertCell(3);
                cell.className  = "select";
          
                cell.appendChild(span);
                cell = row.insertCell(4);
            }

            cell.className = "scan";
            cell.innerHTML = '<input type="button" addr="' + this.addrs[n].address + '" value="rescan" onclick="MyAccount.scan_addr($(this).attr(\'addr\')); var img=document.createElement(\'img\'); img.style.width = \'64px\'; img.src=\'/assets/img/loading.gif\'; $(this).parent().html(img);   "  />';
        }
        this.AddrTable.style.display = 'block';
    }
        
    $('#addr-total').html(total_amount/unit);
}

AccountList.prototype.scan_account = function () {
    var self = this;
    
    var AddrAr = [];
    
    for (var n = 0; n < this.addrs.length; n++) {
        AddrAr.push(this.addrs[n].address);
    }

    var shownull = $('#show_null').is(':checked') ? true : false;

    $('#account_addresses').css('display', 'none');
    $('#account_addresses_loading').html('<img src="/assets/img/loading.gif" width="64" />');

    rpc_call('rescanaddrs', [AddrAr], function (data) {
    
        rpc_call('getpubaddrs', [self.accountName, shownull], function (data) {

            $('#account_addresses').css('display', 'block');
            $('#account_addresses_loading').html('');

            self.update_addrs();
            for (var n = 0; n < self.accountSelects.length; n++) {
                self.update_addrs_select(self.accountSelects[n]);
            }
        });
    
    });
}

AccountList.prototype.scan_addr = function (address) {
    var self = this;
    var shownull = $('#show_null').is(':checked') ? true : false;

    rpc_call('rescanaddrs', [[address]], function (data) {


        rpc_call('getpubaddrs', [self.accountName, shownull], function (data) {


            if ((typeof data.result.addrs === 'undefined') || (data.result.addrs.length == 0)) {
                self.addrs = null;
            }
            else {
                self.addrs = data.result.addrs;
            }
            self.update_addrs();

            for (var n = 0; n < self.accountSelects.length; n++) {
                self.update_addrs_select(self.accountSelects[n]);
            }

        });

    });
}

AccountList.prototype.find_account = function (accnt_name)
{
    if (this.my_accounts == null) return null;

    for (var n = 0; n < this.my_accounts.length; n++) {

        if (this.my_accounts[n].name == accnt_name)
            return this.my_accounts[n];
    }
    return null;
}

AccountList.prototype.accountselected = function (accnt_name)
{

    

    if (this.unspents != null) {
        this.unspents = null;
        this.update_unspent();
    }

    if ((accnt_name==null)||(accnt_name.length == 0))
    {
        var n;
        this.accountName = '';

        $('#account_infos').css('display', 'none');
        $('#transaction').css('display', 'none');
        $('#staking').css('display', 'none');
       
        
        
        this.create_addr_table();

        this.p.style.display = 'none';

        if (!this.opts.newAccnt) {
            this.input.style.display = 'none';

            $('#newKeyBut').css('display', 'none');
            $('#newaddr').css('display', 'none');
            $('#account-label').css('display', 'none');
            $('#account-label').val('');
            
        }
        else {
            this.input.disabled = false;
            $('#newaddr').css('display', 'block');
            $('#newKeyBut').css('display', 'block');
            $('#account-label').css('display', 'block');
            this.input.style.display = 'inline';
        }

        this.input.value      = '';
        this.addrs            = null;
        this.update_addrs();

        for (n = 0; n < this.accountSelects.length; n++) {
            this.update_addrs_select(this.accountSelects[n]);
        }
    }
    else
    {
        var n;
        var self = this;

        $('#account_infos').css('display', 'block');
        $('#transaction').css('display', 'block');
        $('#newKeyBut').css('display', 'block');
        
        this.p.style.display = 'block';

        var shownull = $('#show_null').is(':checked') ? true : false;

        this.accountName = accnt_name;
        this.input.disabled = true;
        this.input.value = this.accountName;
        this.input.style.display = 'inline';

        $('#account-label').css('display','none');
        

        $('#account_addresses').css('display', 'none');
        $('#account_addresses_loading').html('<img src="/assets/img/loading.gif" width="64" />');


        rpc_call('getpubaddrs', [this.accountName, shownull], function (data) {


            $('#account_addresses').css('display', 'block');
            $('#account_addresses_loading').html('');

            $('#newaddr').css('display', 'block');
            $('#rescan-all').css('display', 'block');

            if ((typeof data.result.addrs === 'undefined') || (data.result.addrs.length == 0)) {
                self.addrs = null;
            }
            else {
                self.addrs = data.result.addrs;
            }
            self.update_addrs();

            for (n = 0; n < self.accountSelects.length; n++)
            {
                self.update_addrs_select(self.accountSelects[n]);
            }

            if (self.accountName == 'anonymous')
            {
                $('#sendtx_but').prop('disabled', true);
                $('#sendtx_but').attr('display', 'none');
                $('#viewPrivSecret').prop('disabled', true);
                $('#maketx').html('send tx');
                $('#staking').css('display', 'none');
            }
            else
            {
                if (self.opts.staking == true)
                    $('#staking').css('display', 'block');

                $('#sendtx_but').prop('disabled', false);
                $('#sendtx_but').removeProp('disabled');
                $('#sendtx_but').attr('display', 'block');
                
                $('#viewPrivSecret').prop('disabled', false);
                $('#viewPrivSecret').removeProp('disabled');
                $('#maketx').html('view tx');
            }
        }, function () { $('#account_addresses_loading').html('error');  });
    }
}

AccountList.prototype.setAccounts = function (accounts,accountName)
{
    var self = this;
    var n, selectedIdx;
    var opt;


    //this.input.display = 'inline';


    while (this.select.options.length) {
        this.select.remove(0);
    }

    if (this.opts.newAccnt) {
        opt         = document.createElement('option');
        opt.text    = 'new account';
        opt.value   = '';
        this.select.add(opt);
    }
    else
    {
        opt         = document.createElement('option');
        opt.text    = 'select an account';
        opt.value   = '';
        this.select.add(opt);
    }

    if ((accounts == null) || (accounts.length == 0)) {

        this.accountName = '';
        return;
    }

    selectedIdx = -1;

    for (n = 0; n < accounts.length; n++) {
        opt         = document.createElement('option');
        opt.text    = accounts[n].name;
        opt.value   = accounts[n].name;

        this.select.add(opt);

        if (accountName == accounts[n].name)
            selectedIdx = n+1;
    }
    if (selectedIdx >= 0) {
        this.select.selectedIndex = selectedIdx;
    }

    this.accountselected(accountName);

    this.select.addEventListener("change", function () {
        if (self.opts.redirect) {
            window.location.href = self.opts.redirect + this.value;
        }
        else
            self.accountselected(this.value);
    });
}
    
AccountList.prototype.updateAccounts = function ()
{
    var self = this;

    rpc_call('listaccounts', [0], function (data) {

        if ((typeof data.result.accounts == 'undefined') || (data.result.accounts.length == 0)) {
            self.my_accounts = null;
        }
        else
        {
            self.my_accounts = data.result.accounts;
            self.setAccounts(self.my_accounts);
        }
    });
}


AccountList.prototype.addAccountSelect = function (select_name)
{
    this.accountSelects.push(select_name);
}


AccountList.prototype.setAddressText = function (text)
{
    this.p.innerHTML = text;
}


AccountList.prototype.import_keys = function (label)
{
    var self = this;
    var arrKey;
    var encKey;
    var secret;
    var acName,acVal;
    var hexKey, HexKey;
    
    if (key == null) {
        $('#imp_key_msg').empty();
        $('#prv_key_msg').html('enter a private key');
        return false;
    }
    secret = $('#imp_key').val();
    if (secret.length < 6) {
        $('#imp_key_msg').html('key too short (min 6 cars)');
        $('#prv_key_msg').empty();
        return false;
    }
    
    acVal = $('#my_account').val();
    
    if (acVal.length <= 0)
        acVal = $('#account_name').val();

 
    
    if (acVal.length <= 0) {
        alert('Please select an account or create a new one.')
        return;
    }
    
    this.accountName = acVal;
    
    
    acName = this.accountName.replace('@', '-');
    arrKey = key.getPrivate().toArray('be', 32);
    encKey = rc4_cypher_arr(secret, arrKey);
    HexKey = strtoHexString(encKey);
    
    rpc_call('importkeypair', [acName, label, pubkey, HexKey, 0], function (data) {
        var n;
        $('#prv_key_msg').empty();
        $('#imp_key_msg').empty();
    
        self.accountselected(self.accountName);
    
        for (n = 0; n < self.select.options.length; n++)
        {
            if (self.select.options[n].value == self.accountName)
            {
                self.select.selectedIndex = n;
            }
        }
    });
    
    return true;
}

AccountList.prototype.makeModal = function (modal_id)
{
    var modal,dialog,content,body,inner;
    
    modal=document.createElement('div');
    dialog=document.createElement('div');
    content=document.createElement('div');
    body=document.createElement('div');
    inner=document.createElement('div');
    
    modal.className='modal fade top modal-content-clickable';
    modal.tabIndex=-1;
    modal.id = modal_id;
    modal.setAttribute('role','dialog');
    modal.setAttribute('aria-labelledby','myModalLabel');
    modal.setAttribute('data-backdrop','false');
    modal.setAttribute('aria-hidden','true');
    modal.style.display='none';
    
    dialog.className='modal-dialog modal-frame modal-top modal-notify modal-info';
    dialog.setAttribute('role','document');
    
    content.className='modal-content';
    body.className='modal-body';
    inner.className='text-center';
    
    
    body.appendChild(inner);
    content.appendChild(body);
    dialog.appendChild(content);
    modal.appendChild(dialog);
    document.body.appendChild(modal);
    
    return inner;
}

AccountList.prototype.newAddrForm = function ()
{
    var h3,a,inner,input,label,form,span;
    
    form = document.createElement('form');
    
    h3=document.createElement('h3');
    h3.innerHTML='import';
    
    form.appendChild(h3);
    
    inner = document.createElement('div');
    inner.className = 'md-form';
    
    input = document.createElement('button');
    input.className = 'btn btn btn-primary';
    input.type = "button";
    input.onclick = function () { generateKeys(); }
    input.innerHTML = 'create new address';
    
    
    inner.appendChild(input);
    form.appendChild(inner);
    
    inner = document.createElement('div');
    inner.className = 'form-group row';
    
    label = document.createElement('label');
    label.className = 'col-form-label';
    label.setAttribute('for', 'addrlabel');
    label.innerHTML = 'Key label :';
    
    input = document.createElement('input');
    input.id = "addrlabel";
    input.name = "addrlabel";
    input.type = "text";
    input.className = 'form-control';
    input.setAttribute('placeholder', 'new address');
    
    inner.appendChild(label);
    inner.appendChild(input);
    form.appendChild(inner);
    
    inner = document.createElement('div');
    inner.className = 'form-group row';
    
    label = document.createElement('label');
    label.className = 'col-form-label';
    label.setAttribute('for', 'pubaddr');
    label.innerHTML = 'public address';
    
    input = document.createElement('input');
    input.id = "pubaddr";
    input.name = "pubaddr";
    input.type = "text";
    input.className = 'form-control';
    input.setAttribute('disabled','disabled');
    
    inner.appendChild(label);
    inner.appendChild(input);
    form.appendChild(inner);
    
    inner = document.createElement('div');
    inner.className = 'md-form';
    
    label = document.createElement('label');
    label.setAttribute('for', 'pubaddr');
    label.innerHTML = 'private address';
    
    input = document.createElement('input');
    input.id = "privkey";
    input.name = "privkey";
    input.type = "text";
    input.className = 'form-control';
    input.addEventListener("input", function () { newkey(this.value);});
    
    inner.appendChild(input);
    inner.appendChild(label);
    form.appendChild(inner);
    
    span=document.createElement('prv_key_msg');
    span.style.color='red';
    form.appendChild(span);
    
    inner = document.createElement('div');
    inner.className = 'md-form';
    
    label = document.createElement('label');
    label.setAttribute('for', 'pubaddr');
    label.innerHTML = 'secret key';
    
    input = document.createElement('input');
    input.id = "imp_key";
    input.name = "imp_key";
    input.type = "text";
    input.className = 'form-control';
    
    
    inner.appendChild(input);
    inner.appendChild(label);
    form.appendChild(inner);
    
    inner = document.createElement('div');
    inner.className = 'md-form';
    
    input = document.createElement('button');
    input.className = 'btn btn-primary';
    input.type = "button";
    input.id = 'import_btn';
    input.addEventListener('click', function () { MyAccount.import_keys($('#addrlabel').val()); });
    input.innerHTML = 'import';


    inner.appendChild(input);
    form.appendChild(inner);
    
    span = document.createElement('div');
    span.id='imp_key_msg';
    span.style.color='red';
    form.appendChild(span);
    
    return form
}

AccountList.prototype.addr_selected = function(addr)
{
    $('#unspentaddr').html(addr);
}

AccountList.prototype.update_timeout = function () {
    var time = parseInt($('#anon_timeout').val());
    var self = this;
    if (time >= 1) {
        time--;
        $('#anon_timeout').val(time);
        this.AnonTimeout = setTimeout(function () { self.update_timeout(); }, 1000);
    }
    else
        $('#anon_locked').html('<span  class="mdi mdi-lock" style="color:red">locked</span >');
}

AccountList.prototype.check_anon_access = function () {
    var self = this;

    anon_rpc_call('accesstest', [], function (data) {

        if (data.error)
            anon_access = false;
        else
            anon_access = true;

        if (anon_access) {
            $('#anon_wallet').css('display', 'block');
            $('#enable_staking').prop("disabled", false);

            if (data.result.unlocked) {

                $('#anon_timeout').val(data.result.time);
                $('#anon_locked').html('<span class="mdi mdi-lock-open" style="color:green">unlocked</span >');

                if (data.result.staking)
                    $('#enable_staking').prop("checked", true);
                else
                    $('#enable_staking').prop("checked", false);

                if (self.AnonTimeout != null) clearTimeout(self.AnonTimeout);

                self.AnonTimeout = setTimeout(function (){ self.update_timeout(); }, 1000);

            }
            else {
                $('#anon_timeout').val(1800);
                $('#anon_locked').html('<span  class="mdi mdi-lock" style="color:red">locked</span >');
         0.   }
        }
        else {
            $('#anon_wallet').css('display', 'none');
            $('#enable_staking').prop("disabled", "disabled");
        }
    }, function () { anon_access = false; });
}


AccountList.prototype.set_anon_pw = function (pw, timeout, staking) {
    var self = this;

    $('#anon_pw_error').empty();
    $('#anon_pw_ok').html('<img src="/assets/img/loading.gif" width="64" />');
    

    anon_rpc_call('walletpassphrase', [pw, timeout, staking], function (data) {

        $('#anon_pw_ok').empty();

        if (data.error) {
            $('#enable_staking').prop('checked', false);
            $('#anon_pw_error').html('request error');
        }
        else {
            if (data.result.pw) {
                if (self.AnonTimeout != null)
                    clearTimeout(self.AnonTimeout);

                if (timeout>0){
                    self.AnonTimeout = setTimeout(function () { self.update_timeout(); }, 1000);
                    $('#anon_locked').html('<i class="mdi mdi-lock-open" style="color:green">unlocked</i>');
                }
                else
                    $('#anon_locked').html('<i  class="mdi mdi-lock" style="color:red">locked</i >');

                $('#anon_pw_error').empty();
               
            }
            else {
                $('#anon_pw_error').html('wrong password');
            }

            if (data.result.staking === true)
                $('#enable_staking').prop('checked', 'checked');
            else
                $('#enable_staking').prop('checked', false);
        }
    }, function () {
        $('#anon_pw_ok').empty();
        $('#enable_staking').prop('checked', false);
        $('#anon_pw_error').html('request error');
    });
}

function sign_hash(username,addr,secret,sign_data) {

    var acName = username.replace('@', '-');

    $('#bounty_submit').attr('disabled', 'disabled');

    if (sign_data.length == 0) {
        $('#bounty_sig_msg').html('<p style="color:red;">empty twid id</p>');
        return false;
    }
    if (secret.length < 6) {
        $('#bounty_sig_msg').html('<p style="color:red;">secret too short</p>');
        return false;
    }

    rpc_call('getprivaddr', [acName, addr], function (keyData) {

        var DecHexkey = strtoHexString(un_enc(secret, keyData.result.privkey.slice(0,64)));
        var pubkey;

        key     = ec.keyPair({ priv: DecHexkey, privEnc: 'hex' });
        pubkey  = key.getPublic().encodeCompressed('hex');
        
        var maddr = pk2addr(tpubkey);

        //rpc_call('pubkeytoaddr', [pubkey], function (data) {

        if (maddr == $('#bounty_addr').val()) {
                var signature = key.sign(sign_data);
                // Export DER encoded signature in Array
                var derSign = signature.toDER('hex');

                $('#bounty_secret').val ('');

                $('#bounty_pubkey').val (pubkey);
                $('#bounty_key').html   (DecHexkey);
                $('#bounty_sig').val    (derSign);
                $('#bounty_sig_msg').html('<p style="color:green;">address check ok</p>');

                $('#bounty_submit').removeAttr('disabled');
            }
            else
            {
                $('#bounty_pubkey').val('');
                $('#bounty_key').val('');
                $('#bounty_sig').val('');
                $('#bounty_sig_msg').html('<p style="color:red;">address check err</p>');
            }

        });
    //});
}







function txinputsigned(txsign) {
    nSignedInput++;
    if (nSignedInput >= nInputTosign) {
        var txid = txsign.result.txid;
        rpc_call('submittx', [txid], function () { });
        $('#newtxid').html(txid);
        $('#newtx').empty();
        $('#type_tx').empty();
        nSignedInput = -1;
    }

}

function signtxinputs(txh, inputs) {
    nSignedInput = 0;
    nInputTosign = inputs.length;

    for (var n = 0; n < inputs.length; n++) {

        if ((inputs[n].isApp == true) || ((inputs[n].srcapp) && (!inputs[n].isAppType) && (!inputs[n].isAppObj) && (!inputs[n].isAppLayout) && (!inputs[n].addChild) && (!inputs[n].isAppModule))) {
            nInputTosign--;

            if (nInputTosign == 0)
            {
                rpc_call('submittx', [txh], function () { });
                return;
            }
        }
        else {
            var DecHexkey = $('#selected_' + inputs[n].srcaddr).attr('privkey');
            var pubKey = $('#selected_' + inputs[n].srcaddr).attr('pubkey');
            var mykey = ec.keyPair({ priv: DecHexkey, privEnc: 'hex' });
            var signature = mykey.sign(inputs[n].signHash, 'hex');
            // Export DER encoded signature in Array
            //var derSign = signature.toDER('hex');
            var derSign = signature.toLowS();
            
            my_tx                       = null;
            $('#div_newtxid').css('display', 'block');
            $('#sendtx_but').prop("disabled", true);

            rpc_call('signtxinput', [txh, inputs[n].index, derSign, pubKey], txinputsigned);
        }
    }
}



var inputsToSign = [];
var txSignPromise = null;
var signTxHash;
var curInput = null;

function signtxinput_cb(data) {

    var input = inputsToSign.pop();

    if(data != null)
        signTxHash = data.result.txid;


    if (curInput != null)
    {
        console.log('tx_input_' + curInput.index);

        $('#tx_input_' + curInput.index).attr('class', 'mdi mdi-lock');
        $('#tx_input_' + curInput.index).css('color', 'green');
    }

    curInput = input;

    if (input == null) {
        $('#newtx').empty();
        $('#type_tx').empty();

        if ((data == null)||(data.error))
        {
            if (txSignPromise != null)
                txSignPromise.reject();
            return;
        }

        $('#newtxid').html(signTxHash);

        rpc_call('submittx', [signTxHash], function () {
            if (txSignPromise != null)
                txSignPromise.resolve(signTxHash);
        });
        return;
    }
    var DecHexkey = $('#selected_' + input.srcaddr).attr('privkey');
    var pubKey = $('#selected_' + input.srcaddr).attr('pubkey');
    var mykey = ec.keyPair({ priv: DecHexkey, privEnc: 'hex' });
    var signature = mykey.sign(input.signHash, 'hex');
    // Export DER encoded signature in Array
    //var derSign = signature.toDER('hex');
    var derSign = signature.toLowS();

   

    rpc_call_promise('signtxinput', [signTxHash, input.index, derSign, pubKey]).done(signtxinput_cb);
}

function signtxinputs_promise(txh, inputs, done, error) {

    var prom;
    my_tx = null;
    $('#div_newtxid').css('display', 'block');
    $('#sendtx_but').prop("disabled", true);

    inputsToSign = new Array();
    txSignPromise = {};

    txSignPromise.resolve = done;
    txSignPromise.reject = error;

    signTxHash = txh;

    for (var n = 0; n < inputs.length; n++) {

        if ((inputs[n].isApp == true) || ((inputs[n].srcapp) && (!inputs[n].isAppType) && (!inputs[n].isAppObj) && (!inputs[n].isAppLayout) && (!inputs[n].addChild) && (!inputs[n].isAppModule))) {

        }
        else {
            inputsToSign.push(inputs[n]);
        }
    }

    if (inputsToSign.length > 0) {

        curInput = null;
        signtxinput_cb();
    }
    else {
        rpc_call('submittx', [signTxHash], function () { txSignPromise.resolve(signTxHash); });
    }

    return txSignPromise;

}

function maketxfrom(address, amount, dstAddr) {
    var addrs;

    $('#div_newtxid').css('display','none');

    if ((typeof address == 'array') || (typeof address == 'object'))
        addrs = address;
    else
        addrs = [address];

    rpc_call('maketxfrom', [addrs, amount, dstAddr], function (data) {
        my_tx = data.result.transaction;

        $('#sendtx_but').prop("disabled", false);
        $('#total_tx').html(data.result.total);
        $('#newtx').html(get_tmp_tx_html(my_tx));
    });
}

