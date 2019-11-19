/* Write here your custom javascript codes */

var cgi_base = '/api/'
var rpc_base = '/jsonrpc'
var anon_rpc_base = '/pubwal'
var api_base_url = '';
var site_base_url = '';
var sessionid = null;

if (!Uint8Array.prototype.slice && 'subarray' in Uint8Array.prototype)
    Uint8Array.prototype.slice = Uint8Array.prototype.subarray;

var hexChar = ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F"];
var b64s = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"';
var ALPHABET, ALPHABET_MAP, i;
ALPHABET = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
ALPHABET_MAP = {};

i = 0;
while (i < ALPHABET.length) {
    ALPHABET_MAP[ALPHABET.charAt(i)] = i;
    i++;
}

function isHexStr(inputString) {
    var re = /[0-9A-Fa-f]{6}/g;
    return re.test(inputString);
}


function encryptNodeKey(msg)
{   
    var mykey, myprivX;

    if (typeof (ec) == 'undefined')
        return null;

    if (ec == null)
        return null;
    
    myprivX = sessionStorage.MyNodixKey;

    if(!myprivX)
    {
        mykey           = ec.genKeyPair();
        myprivX         = mykey.getPrivate('hex');
        sessionStorage.MyNodixKey = myprivX;
    }
    else
    {
         mykey = ec.keyPair({ priv: myprivX, privEnc: 'hex' });
    }

    
    $('#apiKeys').css('display', 'block');
    $('#nodePubKey-nav').html(NodePubKey);
    $('#myPubKey-nav').html(mykey.getPublic().encodeCompressed('hex'));

    var mypub       = hex2b(mykey.getPublic().encodeCompressed('hex'));
    var theirkey    = ec.keyPair({ pub: NodePubKey, pubEnc: 'hex' });
    var theirpub    = theirkey.getPublic();
    var thekey      = mykey.derive(theirpub);
    var EArr = rc4_cypher_ak(hex2b(thekey.toString(16)), msg);

    return { 'pubkey': to_b58(mypub), 'msg': to_b58(EArr) };

}


function rpc_call(in_method, in_params, in_success, in_error) {

    var encMsg = encryptNodeKey(JSON.stringify(in_params));

    if (encMsg)
        obj = { jsonrpc: '2.0', method: in_method, pubkey: encMsg.pubkey, params: encMsg.msg, id: 1 };
    else
        obj = { jsonrpc: '2.0', method: in_method, params: in_params, id: 1 };
    

    $.ajax({
        url: api_base_url + rpc_base,
        data: JSON.stringify(obj),  // id is needed !!
        contentType: "application/json; charset=utf-8",
        type: "POST",
        dataType: "json",
        success: in_success,
        error: in_error
    });
}

function rpc_call_promise(in_method, in_params, noenc) {

    if (noenc == true) {
        obj = { jsonrpc: '2.0', method: in_method, params: in_params, id: 1 };
    }
    else {
        var encMsg = encryptNodeKey(JSON.stringify(in_params));
        if (encMsg)
            obj = { jsonrpc: '2.0', method: in_method, pubkey: encMsg.pubkey, params: encMsg.msg, id: 1 };
        else
            obj = { jsonrpc: '2.0', method: in_method, params: in_params, id: 1 };
    }

    var promise = $.ajax({
        url: api_base_url + rpc_base,
        data: JSON.stringify(obj),  // id is needed !!
        contentType: "application/json; charset=utf-8",
        type: "POST",
        dataType: "json"
    });

    return promise;
}


function anon_rpc_call(in_method, in_params, in_success,in_error) {

    var encMsg = encryptNodeKey(JSON.stringify(in_params));

    if (encMsg) {
        obj = { jsonrpc: '2.0', method: in_method, pubkey: encMsg.pubkey, params: encMsg.msg, id: 1 };
    }
    else
        obj = { jsonrpc: '2.0', method: in_method, params: in_params, id: 1 };


    $.ajax({
        url: api_base_url + anon_rpc_base,
        data: JSON.stringify(obj),  // id is needed !!
        contentType: "application/json; charset=utf-8",
        type: "POST",
        dataType: "json",
        success: in_success,
        error: in_error
    });
}

function api_call(in_method, in_params, in_success) {
    $.get(api_base_url + cgi_base + in_method + in_params,in_success,"json");
}

function reverse(s) {
    var o = '';
    for (var i = s.length - 2; i >= 0; i -= 2) {
        o += s[i];
        o += s[i + 1];
    }
    return o;
}



function hex32(val) {
    val &= 0xFFFFFFFF;
    var hex = val.toString(16).toUpperCase();
    return reverse(("00000000" + hex).slice(-8));
}

function sha256(s) {                      // Requires jsSHA
    var shaObj = new jsSHA("SHA-256", "HEX");
    shaObj.update(s);
    return shaObj.getHash("HEX");
}
function toHexString(arr) {
    var str = '';
    for (var i = 0; i < arr.length ; i++) {
        str += ((arr[i] < 16) ? "0" : "") + arr[i].toString(16);
    }
    return str;
}

function hex2a(hexx) {
    var hex = hexx.toString();//force conversion
    var str = '';
    for (var i = 0; i < hex.length; i += 2)
        str += String.fromCharCode(parseInt(hex.substr(i, 2), 16));
    return str;
}
function hex2b(hexx) {
    var hex = hexx.toString();//force conversion
    var arr = [];
    for (var i = 0; i < hex.length; i += 2)
        arr.push(parseInt(hex.substr(i, 2), 16));
    return arr;
}

function strtoHexString(istr) {
    var str = '';
    for (var i = 0; i < istr.length ; i++) {
        str += ((istr.charCodeAt(i) < 16) ? "0" : "") + istr.charCodeAt(i).toString(16);
    }
    return str;
}

function get_amount_coin()
{
    return Math.ceil(parseFloat(100000000.0) / unit);
}

function compare_hash(h1, h2) {
    //console.log('hashes :' + h1 + ' ' + h2);
    for (bn = h1.length - 2; bn >= 0; bn -= 2) {
        b1 = parseInt(h1.slice(bn, bn + 2), 16);
        b2 = parseInt(h2.slice(bn, bn + 2), 16);
        //console.log('hex :' + b1 + ' ' + b2);
        if (b1 < b2)
            return 1;
        else
            return 0;
    }
}



function textToBase64(t) {
    var r = ''; var m = 0; var a = 0; var tl = t.length - 1; var c
    for (n = 0; n <= tl; n++) {
        c = t.charCodeAt(n)
        r += b64s.charAt((c << m | a) & 63)
        a = c >> (6 - m)
        m += 2
        if (m == 6 || n == tl) {
            r += b64s.charAt(a)
            if ((n % 45) == 44) { r += "\n" }
            m = 0
            a = 0
        }
    }
    return r
}

function base64ToText(t) {
    var r = ''; var m = 0; var a = 0; var c
    for (n = 0; n < t.length; n++) {
        c = b64s.indexOf(t.charAt(n))
        if (c >= 0) {
            if (m) {
                r += String.fromCharCode((c << (8 - m)) & 255 | a)
            }
            a = c >> m
            m += 2
            if (m == 8) { m = 0 }
        }
    }
    return r
}


function Uint8ToBase64(u8Arr) {
    var CHUNK_SIZE = 0x8000; //arbitrary number
    var index = 0;
    var length = u8Arr.length;
    var result = '';
    var slice;
    while (index < length) {
        slice = u8Arr.subarray(index, Math.min(index + CHUNK_SIZE, length));
        result += String.fromCharCode.apply(null, slice);
        index += CHUNK_SIZE;
    }
    return btoa(result);
}

function longToByteArray(/*long*/long) {
    // we want to represent the input as a 8-bytes array
    var byteArray = new Uint8Array(4);

    for (var index = 0; index < byteArray.length; index++) {
        var byte = long & 0xff;
        byteArray[index] = byte;
        long = (long - byte) / 256;
    }

    return byteArray;
};

function to_b58(buffer) {
    var carry, digits, j;
    if (buffer.length === 0) {
        return "";
    }
    i = void 0;
    j = void 0;
    digits = [0];
    i = 0;
    while (i < buffer.length) {
        j = 0;
        while (j < digits.length) {
            digits[j] <<= 8;
            j++;
        }
        digits[0] += buffer[i];
        carry = 0;
        j = 0;
        while (j < digits.length) {
            digits[j] += carry;
            carry = (digits[j] / 58) | 0;
            digits[j] %= 58;
            ++j;
        }
        while (carry) {
            digits.push(carry % 58);
            carry = (carry / 58) | 0;
        }
        i++;
    }
    i = 0;
    while (buffer[i] === 0 && i < buffer.length - 1) {
        digits.push(0);
        i++;
    }
    return digits.reverse().map(function (digit) {
        return ALPHABET[digit];
    }).join("");
};



function from_b58(S,           //Base58 encoded string input
    A             //Base58 characters (i.e. "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz")
) {
    var d = [],   //the array for storing the stream of decoded bytes
        b = [],   //the result byte array that will be returned
        i,        //the iterator variable for the base58 string
        j,        //the iterator variable for the byte array (d)
        c,        //the carry amount variable that is used to overflow from the current byte to the next byte
        n;        //a temporary placeholder variable for the current byte
    for (i in S) { //loop through each base58 character in the input string
        j = 0,                             //reset the byte iterator
        c = A.indexOf(S[i]);             //set the initial carry amount equal to the current base58 digit
        if (c < 0)                          //see if the base58 digit lookup is invalid (-1)
            return undefined;              //if invalid base58 digit, bail out and return undefined
        c || b.length ^ i ? i : b.push(0); //prepend the result array with a zero if the base58 digit is zero and non-zero characters haven't been seen yet (to ensure correct decode length)
        while (j in d || c) {               //start looping through the bytes until there are no more bytes and no carry amount
            n = d[j];                      //set the placeholder for the current byte
            n = n ? n * 58 + c : c;        //shift the current byte 58 units and add the carry amount (or just add the carry amount if this is a new byte)
            c = n >> 8;                    //find the new carry amount (1-byte shift of current byte value)
            d[j] = n % 256;                //reset the current byte to the remainder (the carry amount will pass on the overflow)
            j++                            //iterate to the next byte
        }
    }
    while (j--)               //since the byte array is backwards, loop through it in reverse order
        b.push(d[j]);      //append each byte to the result
    return new Uint8Array(b) //return the final byte array in Uint8Array format
}




function timeConverter(UNIX_timestamp) {
    var offset = new Date().getTimezoneOffset();
    var a = new Date(UNIX_timestamp * 1000);
    var months = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];
    var year = a.getFullYear();
    var month = months[a.getMonth()];
    var date = a.getDate();
    var hour = a.getHours();
    var min = a.getMinutes();
    var sec = a.getSeconds();
    var time = '<span class="block_date">' + date + ' ' + month + ' ' + year + '</span><span class="block_time">' + hour + ':' + min + ':' + sec + '</span>';
    return time;
}


function dateConverter(UNIX_timestamp) {
    var offset = new Date().getTimezoneOffset();
    var a = new Date(UNIX_timestamp * 1000 + (offset * 60000));
    var months = ['01', '02', '03', '04', '05', '06', '07', '08', '09', '10', '11', '12'];
    var year = a.getFullYear();
    var month = months[a.getMonth()];
    var date;

    if (a.getDate() < 10)
        date = '0' + a.getDate();
    else
        date = a.getDate();

    var time = year + '-' + month + '-' + date;
    return time;
}


function twit_id_from_url(url_html_id) {
    var urlsegs, username, funcseg;
    var urlinput = document.getElementById(url_html_id);
    var twit_url = urlinput.value;
    var parser = document.createElement('a');

    parser.href = twit_url;

    urlsegs = parser.pathname.split('/');
    username = urlsegs[1];
    funcseg = urlsegs[2];
    twit_id = urlsegs[3];

    if ((parser.hostname.toLowerCase() != "twitter.com")) {
        return false;
    }
    if ((funcseg.toLowerCase() != "status")) {
        return false;
    }
    $('#tweet_id').val(twit_id);
    $('#tweet_user').val(username);
}


function check_sig(msg, sig, key, parent) {
    if (ec.verify(msg, sig, key, 'hex'))
        $('#' + parent).addClass('checked');
    else
        $('#' + parent).addClass('invalid');
}

function check_hash(twit_id, parent) {
    var bhash = $('#th_' + twit_id).html();
    var bounty = {};

    bounty['tweet_id'] = $('#tid_' + twit_id).html();
    bounty['time'] = $('#time_' + twit_id).attr('time');
    bounty['prevhash'] = $('#ph_' + twit_id).html();
    bounty['user_dir'] = $('#tu_' + twit_id).html();
    bounty['pubkey'] = $('#tpk_' + twit_id).val();
    bounty['signature'] = $('#tsig_' + twit_id).val();
    bounty['reward'] = $('#tr_' + twit_id).html();
    bounty['adm_pubkey'] = $('#tapk_' + twit_id).val();

    var hash_data = bounty['tweet_id'] + bounty['reward'] + bounty['prevhash'] + bounty['time'] + bounty['user_dir'] + bounty['pubkey'] + bounty['signature'] + bounty['adm_pubkey'];
    var hexDat = strtoHexString(hash_data.toString());
    var hash1 = sha256(hexDat);
    var hash2 = sha256(hash1);
    if (hash2 == bhash)
        $('#' + parent + twit_id).addClass('checked');
    else
        $('#' + parent + twit_id).addClass('invalid');

}



function make_var_html(label,val, val_class,label_id)
{
    var html = '<div class="row"><div class="col-sm-2">';

    html +='<label ';

    if((typeof label_id!='undefined'))
       html +='id="' + label_id + '" ';
    html += '>' + label + '</label></div><div class="col-md" style="text-align:left"><span class="' + val_class + '" >' + val + '</span></div></div>';
    return html;
}

function get_tx_html(tx, n) {
    var new_html = '';
    var vin, vout;

    if (typeof tx.vin != 'undefined')
        vin = tx.vin;
    else if(typeof tx.txsin != 'undefined')
        vin = tx.txsin;

    if (typeof tx.vout != 'undefined')
        vout = tx.vout;
    else if (typeof tx.txsout != 'undefined')
        vout = tx.txsout;

    new_html = '<div class="row justify-content-md-center tx_row fade in show" style="border-bottom:1px solid #000;margin-bottom:2px;" onclick="show_tx(\'' + tx.txid + '\');" >';
    new_html += '<div class="col-md-5  align-self-start" >';
    new_html += '<a class="tx_lnk" onclick="SelectTx(\'' + tx.txid + '\'); return false;" href="' + site_base_url + '/tx/' + tx.txid + '">';

    if (typeof (n) != 'undefined') {
        new_html += '#' + n;
    }
    new_html += '</a>';

    if (tx.isNull == true) {
        new_html += '0&nbsp;in&nbsp;';
        new_html += '0&nbsp;out';
    }
    else {
        new_html += vin.length + '&nbsp;in&nbsp;';
        new_html += vout.length + '&nbsp;out';
    }
    new_html += '</div>';
    if (typeof(tx.blockheight)!='undefined') {
        new_html += '<div class="col-md align-self-end" >';
        new_html += 'block #' + tx.blockheight + '&nbsp';
        new_html += '</div>';
    }
    if (tx.blocktime) {
        new_html += '<div class="col-md align-self-end" >';
        new_html += '<span class="block_idate" >' + timeConverter(tx.blocktime) + '</span>';
        new_html += '</div>';
    }
    new_html += '</div>';

    new_html += '<div class="row tx_infos"  style="border-bottom:1px dashed #000;margin-bottom:2px;" id="tx_infos_' + tx.txid + '" >';
    new_html += '<span style="  width: 100%;  display: inline-block;text-align:center" >transaction id :' + tx.txid + '</span><br/>';
    if (tx.isNull == true) {
        new_html += '<div class="col-md" >' + '<h2>inputs</h2>' + '#0 null&nbsp;' + '</div>';
        new_html += '<div class="col-md" >' + '<h2>outputs</h2>' + '#0 null&nbsp;' + '</div>';
    }
    else if (tx.is_app_root == 1)
	{
		new_html += '<div class="col-md" >' + '<h2>inputs</h2>' + '#0&nbsp;approot&nbsp;' + '</div>';
        new_html += '<div class="col-md" >' + '<h2>outputs</h2>' + '#0&nbsp;' + vout[0].value + ' ' + tx.dstaddr + '</div>';
        
    } else {
        new_html += '<div class="col-md" >';
        new_html += '<h2>inputs</h2>';
        if ((tx.isCoinBase == true)) {

            if ((vin) && (vin.length > 0))
                new_html += vin[0].coinbase;
        }
        else {
            var nins, nouts;
            nins = vin.length;
            for (nn = 0; nn < nins; nn++) {
                new_html += '<div class="row">';
                new_html += '<div class="col-md" >';
                new_html += '#' + nn + '&nbsp;';

                if ((vin[nn].isApp == true)) {
                    new_html += '<span class="app_name" >' + vin[nn].appName+ '</span>';
                }
                else
                {
                    if (vin[nn].srcapp) {
                        new_html += 'App&nbsp; : ' + vin[nn].srcapp;

                        if (tx.appChildOf) new_html += '&nbsp; child';
                        if (tx.app_item == 1) new_html += '&nbsp; type';
                        if (tx.app_item == 2) new_html += '&nbsp; obj';
                        if (tx.app_item == 3) new_html += '&nbsp; file';
                        
                    }
                    else {

                        if (typeof vin[nn].objType == 'number') {
                            new_html += '<a href="' + site_base_url + '/address/' + vin[nn].srcaddr + '" class="tx_address" >' + vin[nn].srcaddr + '</a>';
                            new_html += '&nbsp;<span class="objhash" >obj[' + vin[nn].objHash + ']</span>';
                        } else if (typeof vin[nn].srcapp == 'string') {
                            new_html += '<a href="/nodix.site/application/' + vin[nn].srcapp + '" class="app-lnk" >app[' + vin[nn].srcapp + ']</a>';
                        } else if (typeof vin[nn].imvalue == 'number') {
                            new_html += '<span class="ptree_val" >' + vin[nn].imvalue + '</span>';
                        }
                        else if (typeof vin[nn].keyName == 'string') {
                            new_html += '<span class="ptree_var" >' + vin[nn].appName + '&nbsp;object&nbsp;' + vin[nn].objHash + '&nbsp;key&nbsp;' + vin[nn].keyName + '</span>';
                        } else if (typeof vin[nn].var_name == 'string') {
                            new_html += '<span class="ptree_var" >var&nbsp;' + vin[nn].var_name + '</span>';
                        } else if (typeof vin[nn].op_name == 'string') {
                            new_html += '<span class="ptree_op" >operation &nbsp;' + vin[nn].op_name + '</span>';
                        } else if (typeof vin[nn].fn_name == 'string') {
                            new_html += '<span class="ptree_fbn" >function &nbsp;' + vin[nn].fn_name + '</span>';
                        }
                        else if (typeof vin[nn].value == 'number') {
                            if (vin[nn].addresses) {
                                new_html += '<a href= "' + site_base_url + '/address/' + vin[nn].addresses[0] + '" class="tx_address">' + vin[nn].addresses[0] + '</a>';
                            }
                            else if (vin[nn].srcaddr) {
                                new_html += '<a href= "' + site_base_url + '/address/' + vin[nn].srcaddr + '" class="tx_address">' + vin[nn].srcaddr + '</a>';
                            }
                            new_html += '<span class="tx_amnt" >' + vin[nn].value / unit + '</span>';
                        }
                         
                    }
                }
              
                new_html += '</div>';
                new_html += '</div>';
            }
        }
        new_html += '</div>';

        new_html += '<div class="col-md" >';
        new_html += '<h2>outputs</h2>';
        if (vout) {
            nouts = vout.length;
            for (nn = 0; nn < nouts; nn++) {

                new_html += '<div class="row">';
                new_html += '<div class="col-md" >';

                if (vout[nn].isNull == true)
                    new_html += '#0 null &nbsp;';
                else {
                    var val;

                    new_html += '#' + nn + '&nbsp;';

                    if (typeof vout[nn].op_name == 'string') {
                        new_html += '<span class="ptree_op" >operation &nbsp;' + vout[nn].op_name + '</span>';
                    } else if (typeof vout[nn].fn_name == 'string') {
                        new_html += '<span class="ptree_fbn" >function &nbsp;' + vout[nn].fn_name + '</span>';
                    }
                    else if (typeof vout[nn].value == 'number')
                    {
                        var xval = vout[nn].value.toString(16).substring(0, 8);

                        if ((xval.toLowerCase() == 'ffffffff') || (vout[nn].value == 0xFFFFFFFFFFFFFFFF))
                            val = 0;
                        else
                            val = vout[nn].value;

                        if (vout[nn].addresses) {
                            new_html += '<a href="' + site_base_url + '/address/' + vout[nn].addresses[0] + '" class="tx_address" >' + vout[nn].addresses[0] + '</a>';
                        } else if (vout[nn].dstaddr) {
                            new_html += '<a href="' + site_base_url + '/address/' + vout[nn].dstaddr + '" class="tx_address" >' + vout[nn].dstaddr + '</a>';
                        }
                        new_html += '<span class="tx_amnt" >' + val / unit + '</span>';
                    }
                    else if (typeof vout[nn].objType == 'number')
                    {
                        new_html += '<a href="' + site_base_url + '/address/' + vout[nn].dstaddr + '" class="tx_address" >' + vout[nn].dstaddr + '</a>';
                        new_html += '&nbsp;<span class="objhash" >obj[' + vout[nn].objHash + ']</span>';
                    }
                   
                }
                new_html += '</div>';
                new_html += '</div>';
            }
        }
        new_html += '</div>';
    }
    new_html += '</div>';

    return new_html;
}

function get_tmp_tx_html(tx) {
    var new_html = '';
    var vin, vout;
    var nins, nouts;
    if (typeof tx.vin != 'undefined')
        vin = tx.vin;
    else if (typeof tx.txsin != 'undefined')
        vin = tx.txsin;

    if (typeof tx.vout != 'undefined')
        vout = tx.vout;
    else if (typeof tx.txsout != 'undefined')
        vout = tx.txsout;

    if (tx.isNull == true) {
        new_html += '0&nbsp;in&nbsp;';
        new_html += '0&nbsp;out';
    }
    else {
        new_html += vin.length + '&nbsp;in&nbsp;';
        new_html += vout.length + '&nbsp;out';
    }
    
    new_html += '<div class="row"  style="border-bottom:1px dashed #000;margin-bottom:2px;" >';
    
    new_html += '<div class="col-md-6" >';
    new_html += '<h2>inputs</h2>';

    if (vin) {
        nins = vin.length;
        for (nn = 0; nn < nins; nn++) {
            new_html += '<div class="row">';
            new_html += '<div class="col-1 no-padding" ><i id="tx_input_' + vin[nn].index + '" class="mdi mdi-lock-open"></i></div>';
            new_html += '<div class="col-1 no-padding" >#' + vin[nn].index + '</div>';
            new_html += '<div class="col-md-5 no-padding" >';
            if ((vin[nn].isApp == true)) {
                new_html += '<span class="app_name" >' + vin[nn].appName + '</span>';
            }
            else if (vin[nn].srcapp) {
                new_html += 'App&nbsp; : ' + vin[nn].srcapp;
            }
            else {
                new_html += '<a href= "' + site_base_url + '/address/' + vin[nn].srcaddr + '" class="tx_address">' + vin[nn].srcaddr + '</a>';
                new_html += '</div>';
                new_html += '<div class="col-md-5 no-padding" >';
                new_html += '<span class="tx_amnt" >' + vin[nn].value / unit + '</span>';
            }
            new_html += '</div>';
            new_html += '</div>';
        }
    }
    new_html += '</div>';
    
    new_html += '<div class="col-md-6" >';
    new_html += '<h2>outputs</h2>';
    if (vout) {
        nouts = vout.length;
        for (nn = 0; nn < nouts; nn++) {
            var xval = vout[nn].value.toString(16).substring(0, 8);
            var val;
            new_html += '<div class="row">';
            new_html += '<div class="col-1 no-padding">';
            new_html += '#' + nn + '&nbsp;';
            new_html += '</div>';
            new_html += '<div class="col-md-5 no-padding" >';
            new_html += '<a href="' + site_base_url + '/address/' + vout[nn].dstaddr + '" class="tx_address" >' + vout[nn].dstaddr + '</a>&nbsp;';

            if ((xval.toLowerCase() == 'ffffffff') || (val == 0xFFFFFFFFFFFFFFFF))
                val = 0;
            else
                val = vout[nn].value;
            new_html += '</div>';
            new_html += '<div class="col-md-5 no-padding" >';
            new_html += '<span class="tx_amnt" >' + val / unit + '</span>';
            new_html += '</div>';
            new_html += '</div>';
        }
    }
    new_html += '</div>';
    new_html += '</div>';

    return new_html;
}






function show_tx(txid) {
    if ($('#tx_infos_' + txid).css('display') == 'none') {
        $('#tx_infos_' + txid).animate({ display: 'block', height: "toggle" });
    }
    else {
        $('#tx_infos_' + txid).animate({ display: 'none', height: "toggle" });

    }
}

function deleteAllCookies() {
    var cookies = document.cookie.split(";");

    for (var i = 0; i < cookies.length; i++) {
        var cookie = cookies[i];
        var eqPos = cookie.indexOf("=");
        var name = eqPos > -1 ? cookie.substr(0, eqPos) : cookie;
        document.cookie = name + "=;expires=Thu, 01 Jan 1970 00:00:00 GMT";
    }
}
function get_session(account, pw) {

    deleteAllCookies();
    $.getJSON('/siteapi/getsession/' + account + '/' + pw).done(function (data) { $.cookie("sessionid", data.sessionid); location.reload(); }).error(function (data) { $('#pw').css('border-color', '#F00'); });
}

function clear_session() {
    deleteAllCookies();
    if (sessionid == null) return;
    $.getJSON('/siteapi/clearsession/' + sessionid).done(function (data) { sessionid = null;location.reload(); });
}

function set_account_pw(account,pw) {
    rpc_call('setaccountpw', [account, pw], function (data) { if (data.error == 0) $('#pw').css('border-color', '#0F0'); else $('#pw').css('border-color', '#F00'); });
}
