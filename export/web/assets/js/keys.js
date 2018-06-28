var private_prefix = '55';
var key = null;
var my_tx = null;
var paytxfee = 10000;
var nSignedInput = 0;

var anon_access = false;
var stake_infos = {};
var MyAccount = null;


if (!Uint8Array.prototype.slice && 'subarray' in Uint8Array.prototype)
    Uint8Array.prototype.slice = Uint8Array.prototype.subarray;


function pubkey_to_addr(pubkey) {
    rpc_call('pubkeytoaddr', [pubkey], function (data) {
        $('#pubaddr').val(data.result.addr);
    });
}


function privKeyAddr(username, addr, secret) {
    var acName = username.replace('@', '-');

    if (acName == "anonymous") {
        anon_rpc_call('dumpprivkey', [addr], function (keyData) {
            var faddr, paddr, eaddr, crc;
            var hexk = keyData.result.privkey.slice(0, 64);
            var key = ec.keyPair({ priv: hexk, privEnc: 'hex' });
            var xk = key.getPrivate('hex');
            var pub = key.getPublic().encodeCompressed('hex');
            var addr = private_prefix + hexk + '01';

            console.log(xk);
            console.log(hexk);
            console.log(pub);
            console.log(addr);

            h = sha256(addr);
            h2 = sha256(h);
            crc = h2.slice(0, 8);
            faddr = addr + crc;
            paddr = hex2b(faddr);
            eaddr = to_b58(paddr);
            $('#privAddr').html(eaddr);
        });
    }
    else {
        rpc_call('getprivaddr', [acName, addr], function (keyData) {
            var faddr, paddr, eaddr, crc;
            var xk = keyData.result.privkey.slice(0, 64);
            var DecHexkey = strtoHexString(un_enc(secret, xk));
            var addr = private_prefix + DecHexkey + '01';


            console.log(DecHexkey);
            console.log(addr);



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
    data = from_b58(addr, ALPHABET);

    crc = toHexString(data.slice(34, 38));
    sk = data.slice(0, 34);
    hexk = toHexString(sk);
    h = sha256(hexk);
    h2 = sha256(h);
    if (crc != h2.slice(0, 8))
        alert('bad key');

    sk = data.slice(1, 33);
    hexk = toHexString(sk);
    key = ec.keyPair({ priv: hexk, privEnc: 'hex' });
    pubkey = key.getPublic().encodeCompressed('hex');
    privkey = hexk;
    pubkey_to_addr(pubkey);
}


function check_key(privKey, pubAddr) {
    var test_key = ec.keyPair({ priv: privKey, privEnc: 'hex' });
    pubkey = test_key.getPublic().encodeCompressed('hex');
    rpc_call('pubkeytoaddr', [pubkey], function (data) {
        if (data.result.addr != pubAddr) {
            $('#selected_' + pubAddr).prop('checked', false);
            $('#secret_' + pubAddr).css('color', 'red');
        }
        else {
            $('#selected_' + pubAddr).prop('checked', true);
            $('#secret_' + pubAddr).css('color', 'green');
        }
    });
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

    console.log(strKey);
    console.log(secret);

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
    pubkey_to_addr(pubkey);
}

class AccountList {

    addr_secret_change()
    {
        var addr = this.getAttribute('address');

        $('#selected_' + addr).prop  ('checked',false); 
        self.update_unspent          ();
    }

    select_menu(id)
    {
        if (this.selectedMenu == id) return;

        this.selectedMenu = id;

        if (id == "tab_unspents")
            $('#tab_unspents').addClass('selected');
        else
            $('#tab_unspents').removeClass('selected');

        if (id == "tab_spents")
            $('#tab_spents').addClass('selected');
        else
            $('#tab_spents').removeClass('selected');

        if (id == "tab_received")
            $('#tab_received').addClass('selected');
        else
            $('#tab_received').removeClass('selected');
    }


    update_unspent() {
        var total;
        var n,nrows;
        var old_tbody, new_tbody, thead;
        var num_unspents;
        

        old_tbody             = this.TxTable.tBodies[0];
        new_tbody             = document.createElement('tbody');
        this.selectedUnspents = new Array();

        if (this.unspents == null) {
            $('#txtotal').html(0);
            $('#selected_balance').html(0);
            old_tbody.parentNode.replaceChild(new_tbody, old_tbody);
            return;
        }
        num_unspents                        =  this.unspents.length;
        thead                               =  this.TxTable.tHead;
        thead.rows[0].cells[2].innerHTML    = 'from';
                
        this.selected_balance               = 0;
        total                               = 0;
        nrows                               = 0;


        for (n = 0; n < num_unspents; n++) {

            if (!$('#selected_' + this.unspents[n].dstaddr).is(':checked')) continue;
            var cell;
            var naddr;
            var addresses;
            var row                 = new_tbody.insertRow(nrows++);
            
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
            cell.className  = "unspent_tx";
            cell.innerHTML  = this.unspents[n].txid;


            cell            = row.insertCell(2);
            naddr           = this.unspents[n].addresses.length;
            addresses       = '';

            while (naddr--) {
                addresses += this.unspents[n].addresses[naddr] + '<br/>';
            }

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
            var row         = new_tbody.insertRow(nrows++);

            row.className   = 'tx_error';

            cell            = row.insertCell(0);
            cell.className  = "time";
            cell.innerHTML  = timeConverter(this.unspents[n].time);

            cell            = row.insertCell(1);
            cell.className  = "unspent_tx";
            cell.innerHTML  = this.unspents[n].txid;

            cell            = row.insertCell(2);
            naddr           = this.unspents[n].addresses.length;
            addresses       = '';

            while (naddr--) {
                addresses += this.unspents[n].addresses[naddr] + '<br/>';
            }

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
        old_tbody.parentNode.replaceChild   (new_tbody, old_tbody);
    }



    update_staking_infos()
    {
        if ((this.staking_unspents!=null)&&(this.staking_unspents.length > 0)) {
            $('#do_staking').prop('disabled', false);

            this.totalweight = 0;
            for (var n = 0; n < this.staking_unspents.length; n++) {
                this.totalweight += this.staking_unspents[n].weight;
            }
            $('#stakeweight').html(this.totalweight / unit);
            $('#nstaketxs').html(this.staking_unspents.length);
            $('#stake_msg').empty();
        }
        else {
            $('#do_staking').prop('disabled', true);
            $('#stakeweight').html('0');
            $('#nstaketxs').html('0');
            $('#stake_msg').html('no suitable unspent found');
        }
    }

    fetch_staking_unspents() {
        var self = this;
        var n;

        if (this.addrs == null) return;

        if (this.SelectedAddrs.length == 0)
        {
            this.staking_unspents = null;
            this.update_staking_infos();
            return;
        }

        rpc_call('liststaking', [0, 9999999, this.SelectedAddrs], function (data) {
            var     n;

            self.staking_unspents               = data.result.unspents;
            stake_infos.block_target            = data.result.block_target;
            stake_infos.now                     = data.result.now;
            stake_infos.last_block_time         = data.result.last_block_time;

            self.update_staking_infos            ();
        });
    }


    fetch_unspents() {
        var n;
        var self      = this;
        var AddrAr    = [];

        var old_tbody = this.TxTable.tBodies[0];
        var new_tbody = document.createElement('tbody');

        old_tbody.parentNode.replaceChild(new_tbody, old_tbody);

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

    update_spent() {
        var total;
        var thead;
        var thead, old_tbody, new_tbody;
       

        old_tbody = this.TxTable.tBodies[0];

        if (this.spents == null) {
            old_tbody.parentNode.replaceChild(new_tbody, old_tbody);
            return;
        }
        var nrows = 0;
        var num_spents  = this.spents.length;
      
        new_tbody       = document.createElement('tbody');
        thead           = this.TxTable.tHead;

        thead.rows[0].cells[2].innerHTML = 'to';

        total = 0;

        for (var n = 0; n < num_spents; n++) {
            if (!$('#selected_' + this.spents[n].srcaddr).is(':checked')) continue;

            var cell;
            var naddr;
            var row = new_tbody.insertRow(nrows++);
            var addresses;

            total           += this.spents[n].amount;
            row.className   = 'tx_ready';


            cell            = row.insertCell(0);
            cell.className  = "time";
            cell.innerHTML  = timeConverter(this.spents[n].time);

            cell            = row.insertCell(1);
            cell.className  = "spent_tx";
            cell.innerHTML  = this.spents[n].txid;

            cell            = row.insertCell(2);
            cell.className  = "addr_to";
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

        for (var n = 0; n < num_spents; n++) {
            if ($('#selected_' + this.spents[n].srcaddr).is(':checked')) continue;
            var cell;
            var naddr;
            var row = new_tbody.insertRow(nrows++);
            var addresses;


            row.className   = 'tx_error';

            cell            = row.insertCell(0);
            cell.className  = "time";
            cell.innerHTML  = timeConverter(this.spents[n].time);

            cell            = row.insertCell(1);
            cell.className  = "spent_tx";
            cell.innerHTML  = this.spents[n].txid;

            cell            = row.insertCell(2);
            cell.className  = "addr_to";
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

        $('#txtotal').html  (total / unit);
        $('#ntx').html      (num_spents);
       

        old_tbody.parentNode.replaceChild(new_tbody, old_tbody);
    }

    fetch_spents() {
        var self      = this;
        var n;
        var AddrAr    = [];
        var old_tbody = this.TxTable.tBodies[0];
        var new_tbody = document.createElement('tbody');
      
        old_tbody.parentNode.replaceChild(new_tbody, old_tbody);

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

    update_recvs() {
        var old_tbody = this.TxTable.tBodies[0];
        var new_tbody = document.createElement('tbody');
        var thead;


        if (this.recvs == null) {
            old_tbody.parentNode.replaceChild(new_tbody, old_tbody);
            return;
        }
        var nrows     = 0;
        var total     = 0;                
        var num_recvs = this.recvs.length;

        thead = this.TxTable.tHead;
        thead.rows[0].cells[2].innerHTML = 'from';

        for (var n = 0; n < num_recvs; n++) {
            if (!$('#selected_' + this.recvs[n].dstaddr).is(':checked')) continue;

            var cell;
            var naddr;
            var row = new_tbody.insertRow(nrows++);
            var addresses;
           
            row.className    = 'tx_ready';

            cell            = row.insertCell(0);
            cell.className  = "time";
            cell.innerHTML  = timeConverter(this.recvs[n].time);

            cell            = row.insertCell(1);
            cell.className  = "spent_tx";
            cell.innerHTML  = this.recvs[n].txid;

            cell            = row.insertCell(2);
            cell.className  = "addr_from";
            naddr           = this.recvs[n].addresses.length;
            addresses       = '';

            while (naddr--) {
                addresses += this.recvs[n].addresses[naddr] + '<br/>';
            }
            cell.innerHTML = addresses;

            cell = row.insertCell(3);
            cell.className = "addr_amount";
            cell.innerHTML = this.recvs[n].amount / unit;

            cell = row.insertCell(4);
            cell.className = "tx_conf";
            cell.innerHTML = this.recvs[n].confirmations;

            total += this.recvs[n].amount;
        }

        for (var n = 0; n < num_recvs; n++) {
            if ($('#selected_' + this.recvs[n].dstaddr).is(':checked')) continue;

            var cell;
            var naddr;
            var row = new_tbody.insertRow(nrows++);
            var addresses;

            row.className = 'tx_error';

            cell            = row.insertCell(0);
            cell.className  = "time";
            cell.innerHTML  = timeConverter(this.recvs[n].time);

            cell            = row.insertCell(1);
            cell.className  = "spent_tx";
            cell.innerHTML  = this.recvs[n].txid;

            cell            = row.insertCell(2);
            cell.className  = "addr_from";
            naddr           = this.recvs[n].addresses.length;
            addresses       = '';

            while (naddr--) {
                addresses += this.recvs[n].addresses[naddr] + '<br/>';
            }
            cell.innerHTML = addresses;

            cell = row.insertCell(3);
            cell.className = "addr_amount";
            cell.innerHTML = this.recvs[n].amount / unit;

            cell = row.insertCell(4);
            cell.className = "tx_conf";
            cell.innerHTML = this.recvs[n].confirmations;

            total += this.recvs[n].amount;
        }

        $('#txtotal').html  (total / unit);
        $('#ntx').html      (num_recvs);

        old_tbody.parentNode.replaceChild(new_tbody, old_tbody);
    }

    fetch_recvs() {
        var self      = this;
        var n;
        var AddrAr    = [];
        var old_tbody = this.TxTable.tBodies[0];
        var new_tbody = document.createElement('tbody');
        old_tbody.parentNode.replaceChild(new_tbody, old_tbody);

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

            $('#txtotal').html(data.result.total / unit);
            $('#total_tx').html(data.result.ntx);

            
        });
    }

    maketx(amount, fee, dstAddr) {

        $('#div_newtxid').css('display', 'none');

        rpc_call('maketxfrom', [this.SelectedAddrs, amount, dstAddr, fee], function (data) {

            my_tx = data.result.transaction;

            $('#sendtx_but').prop   ("disabled", false);
            $('#total_tx').html     (data.result.total);
            $('#newtx').html        (get_tmp_tx_html(my_tx));
        });
    }

    
    getSelectedAddrs() {
        var n;
        var AddrAr = [];

        for (n = 0; n < this.addrs.length; n++) {
        if ($('#selected_' + this.addrs[n].address).is(':checked'))
            AddrAr.push(this.addrs[n].address);
        }
        return AddrAr;
    }

    update_txs()
    {
        if (this.selectedMenu == 'tab_spents')
            this.fetch_spents();
        else if (this.selectedMenu == 'tab_received')
            this.fetch_recvs();
        else
            this.fetch_unspents();
    }

    check_address(addr)
    {
        var n;

        if (this.opts.withSecret == false)
        {
            this.SelectedAddrs = new Array();

            for (n = 0; n < this.addrs.length; n++) {
                if ($('#selected_' + this.addrs[n].address).is(':checked'))
                    this.SelectedAddrs.push(this.addrs[n].address);
            }

            if (this.addr_selected)
                this.addr_selected(addr);

            this.update_txs();
                
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

                rpc_call('pubkeytoaddr', [tpubkey], function (data) {

                    if (data.result.addr != addr) {
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
                });
            });
        }
    }

    update_addrs_select(select_name) {
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

    update_addrs()
    {
        var self              = this;
        var old_tbody         = this.AddrTable.tBodies[0];
        var new_tbody         = document.createElement('tbody');
        this.selected_balance = 0;

        if ((this.addrs == null) || (this.addrs.length == 0)) {
            this.AddrTable.style.display = 'none';
        }
        else {
            var n, num_addrs = this.addrs.length;

            for (n = 0; n < num_addrs; n++) {
                var cell, span, input;
                var row = new_tbody.insertRow(n);
                
                row.className   = "my_wallet_addr";
                row.setAttribute("addr", this.addrs[n].address);

                row.addEventListener("click", function () { self.addr_selected(this.getAttribute('addr')); });

                cell            = row.insertCell(0);
                cell.setAttribute("title", this.addrs[n].address)
                cell.setAttribute("class", 'addr_label')
                cell.innerHTML = this.addrs[n].label;

                span            = document.createElement('span');
                span.setAttribute("amount", this.addrs[n].amount);
                span.id         = 'balance_' + this.addrs[n].address;
                span.innerHTML  = this.addrs[n].amount / unit;

                cell            = row.insertCell(1);
                cell.className  = "balance_confirmed";
                cell.appendChild(span);

                span            = document.createElement('span');
                span.innerHTML  = this.addrs[n].unconf_amount / unit;

                cell            = row.insertCell(2);
                cell.className  = "balance_unconfirmed";
                cell.appendChild    (span);

                if (this.opts.withSecret)
                {
                    input = document.createElement('input');
                    input.setAttribute("address", this.addrs[n].address);
                    input.type      = "password";
                    input.id        = 'secret_' + this.addrs[n].address;
                    input.value     = '';

                    span           = document.createElement('div');
                    span.appendChild (input);
                
                    cell            = row.insertCell(3);
                    cell.appendChild  (span);


                    input = document.createElement('input');
                    input.type      = "checkbox";
                    input.id        = 'selected_' + this.addrs[n].address;
                    input.value     = '';
                    input.setAttribute("addr", this.addrs[n].address);

                    input.addEventListener("change", function () { self.check_address( this.getAttribute('addr')); });

                    span = document.createElement('div');
                    span.appendChild(input);

                    cell            = row.insertCell(4);
                    cell.className  = "select";
              
                    cell.appendChild(span);


                    cell = row.insertCell(5);
                }
                else
                {
                    input           = document.createElement('input');
                    input.type      = "checkbox";
                    input.id        = 'selected_' + this.addrs[n].address;
                    input.value     = '';
                    input.setAttribute("addr", this.addrs[n].address);

                    input.addEventListener("change", function () { self.check_address( this.getAttribute('addr')); });

                    span = document.createElement('div');
                    span.appendChild(input);

                    cell            = row.insertCell(3);
                    cell.className  = "select";
              
                    cell.appendChild(span);


                    cell = row.insertCell(4);
                }
                    

                input           = document.createElement('input');
                input.type      = "button";
                input.id        = 'selected_' + this.addrs[n].address;
                input.value     = 'rescan';
                input.setAttribute("addr", this.addrs[n].address);

                span = document.createElement('div');
                span.appendChild(input);


                cell.className = "scan";
                cell.appendChild(span);

                    
                cell.innerHTML = '<div><input addr="' + this.addrs[n].address + '" type="button" value="rescan" onclick="MyAccount.scan_addr($(this).attr(\'addr\'))"; value=""  /></div>';
            }
            this.AddrTable.style.display = 'block';
        }
        old_tbody.parentNode.replaceChild(new_tbody, old_tbody);
    }


    scan_addr(address) {
        var self = this;

        rpc_call('rescanaddrs', [[address]], function (data) {

            rpc_call('getpubaddrs', [this.accountName], function (data) {

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

    find_account(accnt_name)
    {
        if (this.my_accounts == null) return null;

        for (var n = 0; n < this.my_accounts.length; n++) {

            if (this.my_accounts[n].name == accnt_name)
                return this.my_accounts[n];
        }
        return null;
    }

    accountselected(accnt_name)
    {
        if (this.unspents != null) {
            this.unspents = null;
            this.update_unspent();
        }

        if ((accnt_name==null)||(accnt_name.length == 0))
        {
            var n;
            var new_tbody    = document.createElement('tbody');
            this.accountName = '';

            if (!this.opts.newAccnt) {
                this.input.style.display = 'none';
                $('#newaddr').css('display', 'none');
            }
            else {
                this.input.disabled = false;
                $('#newaddr').css('display', 'block');
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


            this.accountName         = accnt_name;
            this.input.disabled      = true;
            this.input.value         = this.accountName;
            this.input.style.display = 'inline';
            
            rpc_call('getpubaddrs', [this.accountName], function (data) {

                $('#newaddr').css('display', 'block');

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
            });
        }
    }

    setAccounts(accounts,accountName)
    {
        var self = this;
        var n, selectedIdx;
        var opt;


        this.input.display = 'inline';


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
    
    updateAccounts()
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


    addAccountSelect(select_name)
    {
        this.accountSelects.push(select_name);
    }


    setAddressText(text)
    {
        this.p.innerHTML = text;
    }


    import_keys(label)
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


    constructor(divName,listName,opts) {
        var self = this;
        var n;
        var div, container, row, col1, col2, label, table,h2;
        var ths = ["label", "balance", "uncomfirmed balance"];
        
        if (opts.withSecret)
        {
            ths.push("secret");
            ths.push("select");
        }

        this.opts               = opts;
        this.accountSelects     = new Array();
        this.accountName        = '';
        this.staking_unspents   = null;
        this.unspents           = null;
        this.minConf            = 10;
        this.selectedMenu       = '';
        this.SelectedAddrs      = new Array();

        div                 = document.getElementById(divName);
        container           = document.createElement('div');
        col1                = document.createElement('div');
        col2                = document.createElement('div');
        row                 = document.createElement('div');
        label               = document.createElement('label');
        this.input          = document.createElement('input');

        this.input.id        = "account_name";
        this.input.name      = "account_name";
        this.input.size      = 12;
        this.input.value     = '';


        this.select                     = document.createElement('select');
        this.select.id                  = "my_account";
        this.select.name                = "my_account";
        this.select.style.display       = 'inline';
        this.select.style.maxWidth      = '120px';



        label.innerHTML                = "account";        
        container.className            = "container";
        row.className                  = "row";
        col1.className                 = "col-md-2";
        col2.className                 = "col-md-4";

        col2.appendChild                 (this.select);
        col2.appendChild                 (this.input);
        col1.appendChild                 (label);
        row.appendChild                  (col1);
        row.appendChild                  (col2);
        container.appendChild            (row);


        row                          = document.createElement('div');
        col1                         = document.createElement('div');
        this.p                       = document.createElement('p');

        row.className                = "row";
        col1.className               = "col-md-6";
        this.p.innerHTML             = 'Below are the addresses for your account.'

        col1.appendChild           (this.p);
        row.appendChild            (col1);
        container.appendChild      (row);

        row                         = document.createElement('div');
        col1                        = document.createElement('div');
        this.AddrTable              = document.createElement('TABLE');

        row.className               = "row";
        col1.className              = "col-md-6";
        this.AddrTable.id           = "my_address_list_table";      
        this.AddrTable.className    = "table table-hover";

        var header = this.AddrTable.createTHead();
        var body   = this.AddrTable.createTBody();
        var trow   = header.insertRow(0);     

        for (n = 0; n < ths.length; n++) {
            var th = document.createElement('th');
            th.innerHTML = ths[n];
            trow.appendChild(th);
        }
        
        col1.appendChild        (this.AddrTable);
        row.appendChild         (col1);
        container.appendChild   (row);

        div.appendChild         (container);


        // build transaction list

        if ((listName != null) && (listName.length > 0))
        {
            div           = document.getElementById(listName);
            container     = document.createElement('div');
            row           = document.createElement('div');
            col1          = document.createElement('div');
            h2            = document.createElement('h2');

            container.className = "container";
            
            row.className       = "row";
            col1.className      = "col-md-2";
            h2.id               = "tab_unspents";
            h2.innerHTML        = 'unspent';
            h2.addEventListener("click", function () { self.fetch_unspents(); });
            col1.appendChild    (h2);
            row.appendChild     (col1);
        
            col1                = document.createElement('div');
            h2                  = document.createElement('h2');
            col1.className      = "col-md-2";
            h2.id               = "tab_spents";
            h2.innerHTML        = 'spent';
            h2.addEventListener("click", function () { self.fetch_spents(); });
            col1.appendChild    (h2);
            row.appendChild     (col1);

            col1                = document.createElement('div');
            col1.className      = "col-md-2";
            h2                  = document.createElement('h2');
            h2.id               = "tab_received";
            h2.innerHTML        = 'received';
            h2.addEventListener ("click", function () { self.fetch_recvs(); });
            col1.appendChild    (h2);
            row.appendChild     (col1);
            container.appendChild(row);


            row                     = document.createElement('div');
            col1                    = document.createElement('div');
            row.className           = "row";
            col1.className          = "col-md-6";
            col1.innerHTML          = '<div class="info">total:<span id="txtotal"></span></div>';
            row.appendChild         (col1);
            container.appendChild   (row);

            row                     = document.createElement('div');
            col1                    = document.createElement('div');
            row.className           = "row";
            col1.className          = "col-md-6";
            col1.innerHTML          = '<div class="info">selected:<span id="selected_balance"></span></div>';
            row.appendChild         (col1);
            container.appendChild   (row);

            row                     = document.createElement('div');
            col1                    = document.createElement('div');
            row.className           = "row";
            col1.className          = "col-md-6";
            col1.innerHTML          = '<div class="info">showing:<span id="ntx"></span>/<span id="total_tx"></span></div>';
            row.appendChild     (col1);
            container.appendChild(row);


            row                     = document.createElement('div');
            col1                    = document.createElement('div');
            row.className           = "row";
            col1.className          = "col-md-6";
            this.TxTable            = document.createElement('TABLE');
            this.TxTable.id         = 'tx_list';
            this.TxTable.className  = "table hover";
       
            header                  = this.TxTable.createTHead();
            body                    = this.TxTable.createTBody();
            trow                    = header.insertRow(0);
            ths                     = ["time","tx","from","amount","nconf"];
        
            for (n = 0; n < ths.length; n++) {
                var th          = document.createElement('th');
                th.innerHTML    = ths[n];
                trow.appendChild(th);
            }

            col1.appendChild        (this.TxTable);
            row.appendChild         (col1);
            container.appendChild   (row);
            div.appendChild         (container);
        }
    }
}

AccountList.prototype.addr_selected = function(addr)
{
    $('#unspentaddr').html(addr);
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
        rpc_call('pubkeytoaddr', [pubkey], function (data) {

            if (data.result.addr == $('#bounty_addr').val()) {
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
    });
}


function check_anon_access() {
    anon_rpc_call('accesstest', [], function (data) {

        if (data.error)
            anon_access = false;
        else {
            anon_access = data.result;

            if (anon_access)
                $('#anon_wallet').css('display', 'block');
        }

    });
}


function set_anon_pw(pw,timeout)
{

    $('#anon_pw_error').empty();
    $('#anon_pw_ok').empty();

    anon_rpc_call('walletpassphrase', [pw,timeout], function (data) {
        
        if (data.error)
            $('#anon_pw_error').html('wrong password');
        else
            $('#anon_pw_ok').html('OK');

    });
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

