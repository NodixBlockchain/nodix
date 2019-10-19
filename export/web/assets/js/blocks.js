var unit = 100000000;
var sent = 0;
var recv = 0;

function BlockExplorer() {

    this.blk_filters = [];
    this.blk_page_idx = 0;
    this.tx_page_idx = 0;

    this.CurrentTime = null;
    this.NextTime = null;
    this.PrevTime = null;

    this.lastLastBlock = null;
    this.evtSource = null;
    this.selectedhash = null;
    this.currentAddr = null;
    this.txs = null;
    this.blocks = null;
    this.blkTable = document.getElementById('list_table');
    this.txList = document.getElementById('tx_list');

}

BlockExplorer.prototype.resetblock=function (block) {
    $("#height").empty();
    $("#hash").empty();
    $("#previousblockhash").empty();
    $("#nextblockhash").empty();
    $("#merkleroot").empty();
    $("#confirmations").empty();
    $("#difficulty").empty();
    $("#nonce").empty();
    $("#reward").empty();
    $("#size").empty();
    $("#bits").empty();
    $("#diffhash").empty();
    $("#proofhash").empty();
    $("#stakemodifier2").empty();
    $("#time").empty();
    $("#version").empty();
    $("#txs").empty();
    $("#blockhash").empty();
}


BlockExplorer.prototype.updateblock = function (block) {

    $("#height").html(block.height);
    $("#hash").html(block.hash);
    $("#previousblockhash").html(block.previousblockhash);
    $("#nextblockhash").html(block.nextblockhash);
    $("#merkleroot").html(block.merkleroot);
    $("#confirmations").html(block.confirmations);
    $("#difficulty").html(block.difficulty);
    $("#nonce").html(block.nonce);
    $("#reward").html(block.reward);
    $("#size").html(block.size);
    $("#bits").html('0x' + block.bits.toString(16));
    $("#diffhash").html(block.hbits);
    $("#proofhash").html(block.proofhash);
    $("#stakemodifier2").html(block.stakemodifier2);
    $("#time").html(timeConverter(block.time));
    $("#version").html(block.version);
    $("#txs").empty();
    if (block.tx) {
        for (var n = 0; n < block.tx.length; n++) {
            $("#txs").append('<div onmouseup="MyBlocks.SelectTx(\'' + block.tx[n] + '\');" >' + block.tx[n] + '</div>');
        }
    }
    $("#blockhash").html(block.hash);
}
BlockExplorer.prototype.create_blk_tbl = function ()
{
    var hdr = [{ id: "height_hdr", label: "height" }, { id: "time_hdr", label: "time" }, { id: "reward_hdr", label: "reward" }, { id: "time_hdr", label: "time" }, { id: "ntx_hdr", label: "ntx" }, { id: "size_hdr", label: "size" }];
    var newTable;
    
    newTable = document.createElement('table');
    newTable.id = "list_table";
    newTable.className = "table table-hover";

    var header = newTable.createTHead();
    var trow = header.insertRow(0);

    for (var n = 0; n < hdr.length; n++) {
        var th = document.createElement('th');
        th.innerHTML = hdr[n].label;
        th.id = hdr[n].id;
        trow.appendChild(th);
    }

    newTable.appendChild(document.createElement('tbody'));

    if(this.blkTable != null) {
        this.blkTable.parentNode.replaceChild(newTable, this.blkTable);
    }
    this.blkTable = newTable;
}
BlockExplorer.prototype.update_blocks = function  ()
{
    var self = this;
    var n, nrow, num_blocks;
    
    
    if (this.blocks != null)
        num_blocks = this.blocks.length;
    else
        num_blocks = 0;
    
    if (this.blkTable == null) return;
    
    this.create_blk_tbl();

    
    if (num_blocks == 0) {
        var row = this.blkTable.tBodies[0].insertRow(nrow);
        var cell;
        cell = row.insertCell(0);
        cell.className = "block_info";
        cell.innerHTML = '#none';
    
        cell = row.insertCell(1);
        cell.className = "block_info";
        cell.innerHTML = 0;
    
        cell = row.insertCell(2);
        cell.className = "block_info";
        cell.innerHTML = '';
    
        cell = row.insertCell(3);
    
        cell.className = "block_info staked";
        cell.innerHTML = '---';
    
        cell = row.insertCell(4);
        cell.className = "block_info";
        cell.innerHTML = '0';
    
        cell = row.insertCell(5);
        cell.className = "block_info";
        cell.innerHTML = '0';
    }
    else {
        nrow = 0;
        for (n = 0; n < num_blocks; n++) {
            if (!this.blocks[n].height) continue;

            var row = this.blkTable.tBodies[0].insertRow(nrow);
            row.id = 'block_' + this.blocks[n].hash;
    
            cell = row.insertCell(0);
            cell.className = "block_info block_height";
            cell.setAttribute('data-toggle', "modal");
            cell.setAttribute('data-target', "#blockmodal");
            cell.innerHTML = '#' + this.blocks[n].height;
    
            cell = row.insertCell(1);
            cell.className = "block_info block_idate";
            cell.innerHTML = timeConverter(this.blocks[n].time);
    
            cell = row.insertCell(2);
            cell.className = "block_info";
    
            if (this.blocks[n].isCoinbase)
                cell.innerHTML = this.blocks[n].reward / unit;
            else
                cell.innerHTML = this.blocks[n].reward / unit;
    
            cell = row.insertCell(3);
    
            if (this.blocks[n].isCoinbase) {
                cell.className = "block_info mined";
                cell.innerHTML = 'mined';
            }
            else {
                cell.className = "block_info staked";
                cell.innerHTML = 'staked';
            }
    
            cell = row.insertCell(4);
            cell.className = "block_info";
    
    
            if (this.blocks[n].tx)
                cell.innerHTML = this.blocks[n].tx.length;
            else if (this.blocks[n].nTx)
                cell.innerHTML = this.blocks[n].nTx;
    
            cell = row.insertCell(5);
            cell.className = "block_info";
            cell.innerHTML = this.blocks[n].size;
            nrow++;
        }
    }
    
    $('.block_info').mouseup(function () { var h = $(this).parent().attr('id').slice(6); self.tx_page_idx = 0; self.txs = null; self.selectBlock(h); self.list_block_txs(h); });
}


BlockExplorer.prototype.update_blocks_calendar = function (d, reload)
{
    var m = $("#block_m").val();
    var y = $("#block_y").val();
    
    if (m.length < 2)
        m = '0' + m.toString(10);
    
    if (d < 10)
        d = '0' + d.toString(10);
    
    $("#block_d").val(d);
    
    $('#cal_div').load(site_url + '/ico/get_cal/' + lang + '/' + y + '/' + m + '/' + d);
    
    if (reload) {
    
        this.blk_page_idx = 0;
        this.tx_page_idx = 0;
        this.blocks = null;
        this.txs = null;
    
        this.list_blocks(y + '-' + m + '-' + d);
        this.list_txs(y + '-' + m + '-' + d);
    }
}

    
BlockExplorer.prototype.update_tx = function (tx) {
            
    $('#size').html(tx.size);
    $('#txtime').html(timeConverter(tx.time));
    $('#blocktime').html(timeConverter(tx.blocktime));
    $('#txblock').html('<a href="' + site_base_url + '/block/' + tx.blockhash + '">' + tx.blockhash + '</a>');

    if (tx.isCoinbase == true) {
        $('#coinbase').html(tx.vin[0].coinbase);
        $('#coinbase').addClass('visible');
        $('#coinbaselbl').addClass('visible');

    }
    else {
        $('#coinbase').removeClass('visible');
        $('#coinbaselbl').removeClass('visible');
    }
    $('#txhash').html(tx.txid);

    if (this.txList == null) return;


    if (this.txList.tBodies) {

        var old_tbody = this.txList.tBodies[0];
        var new_tbody = document.createElement('tbody');
        var row = new_tbody.insertRow(0);
        var cell;
        if (tx.isNull == true) {
            cell = row.insertCell(0);
            cell.className = "txins";
            cell.innerHTML = '#0 null <br/>';
            cell = row.insertCell(1);
            cell.className = "txouts";
            cell.innerHTML = '#0 null <br/>';
        }
        else {
            cell = row.insertCell(0);
            cell.className = "txins";
            if (tx.isCoinBase == false) {
                var nins, nouts;
                var html = '';

                nins = tx.vin.length;
                for (nn = 0; nn < nins; nn++) {
                    html += '#' + tx.vin[nn].n + '<a class="tx_address" href="' + site_base_url + '/address/' + tx.vin[nn].addresses[0] + '">' + tx.vin[nn].addresses[0] + '</a>' + '</span>' + tx.vin[nn].value / unit + '  <br/>';
                }
                cell.innerHTML = html;
            }
            else {
                cell.innerHTML = 'coin base' + tx.vin[0].coinbase;
            }
            cell = row.insertCell(1);
            cell.className = "txouts";

            html = '';
            nouts = tx.vout.length;
            for (nn = 0; nn < nouts; nn++) {
                if (tx.vout[nn].isNull == true)
                    html += '#0 null <br/>';
                else
                    html += '#' + tx.vout[nn].n + '<a class="tx_address" href="' + site_base_url + '/address/' + tx.vout[nn].addresses[0] + '">' + tx.vout[nn].addresses[0] + '</a>' + '</span> ' + tx.vout[nn].value / unit + ' <br/>';
            }
            cell.innerHTML = html;
        }

        old_tbody.parentNode.replaceChild(new_tbody, old_tbody);
    }
    else {
        this.txList.innerHTML = get_tx_html(tx, 0);
    }
}

BlockExplorer.prototype.update_addr_txs = function () {
    var balance = 0;
    var n;
    
    recv = 0;
    sent = 0;
    
    if (this.txList.tBodies) {
    
        var old_tbody = this.txList.tBodies[0];
        var new_tbody = document.createElement('tbody');
    
        for (n = 0; n < this.txs.length; n++) {
            var row = new_tbody.insertRow(n * 2);
            row.className = "txhdr";
    
            cell = row.insertCell(0);
            cell.className = "tx_hash";
            cell.innerHTML = '<a id="addr_tx_"' + this.txs[n].txid + '" href="' + site_base_url + '/tx/' + this.txs[n].txid + '">' + this.txs[n].txid + '</a>';
    
    
            cell = row.insertCell(1);
            cell.className = "txmine";
            cell.innerHTML = 'mined on ' + timeConverter(this.txs[n].blocktime);
    
            row = new_tbody.insertRow(n * 2 + 1);
    
            if (this.txs[n].isNull == true) {
                cell = row.insertCell(0);
                cell.className = "txins";
                cell.innerHTML = '#0 null <br/>';
                cell = row.insertCell(1);
                cell.className = "txouts";
                cell.innerHTML = '#0 null <br/>';
            }
            else {
                cell = row.insertCell(0);
                cell.className = "txins";
                if (this.txs[n].isCoinBase == false) {
                    var nins, nouts;
                    var html = '';
    
                    nins = this.txs[n].vin.length;
                    for (nn = 0; nn < nins; nn++) {
                        html += '#' + this.txs[n].vin[nn].n;
    
                        if (this.txs[n].vin[nn].addresses) {
                            if (this.txs[n].vin[nn].addresses.indexOf(this.currentAddr) >= 0)
                                sent += this.txs[n].vin[nn].value;
    
                            html += ' <a href="' + this.txs[n].vin[nn].addresses[0] + '" class="tx_address">' + this.txs[n].vin[nn].addresses[0] + '</a> ';
                        }
    
                        html += this.txs[n].vin[nn].value / unit + '  <br/>';
                    }
                    cell.innerHTML = html;
                }
                else if ((this.txs[n].vin) && (this.txs[n].vin.length > 0)) {
                    cell.innerHTML = 'coin base' + this.txs[n].vin[0].coinbase;
                }
    
                cell            = row.insertCell(1);
                cell.className  = "txouts";
    
                html            = '';
    
                if (this.txs[n].vout) {
                    nouts = this.txs[n].vout.length;
                    for (nn = 0; nn < nouts; nn++) {
                        if (this.txs[n].vout[nn].isNull == true)
                            html += '#0 null <br/>';
                        else {
                            html += '#' + this.txs[n].vout[nn].n;
    
                            if (this.txs[n].vout[nn].addresses) {
                                if (this.txs[n].vout[nn].addresses.indexOf(this.currentAddr) >= 0)
                                    recv += this.txs[n].vout[nn].value;
    
                                html += ' <a href="' + this.txs[n].vout[nn].addresses[0] + '" class="tx_address">' + this.txs[n].vout[nn].addresses[0] + '</a> ';
                            }
                            html += this.txs[n].vout[nn].value / unit + ' <br/>';
                        }
                    }
                }
                cell.innerHTML = html;
            }
        }
        old_tbody.parentNode.replaceChild(new_tbody, old_tbody);
    }
    else {
        var new_html = '';
        for (n = 0; n < this.txs.length; n++) {
            new_html += get_tx_html(this.txs[n], n);
        }
        this.txList.innerHTML = new_html;
    }
}

BlockExplorer.prototype.update_txs = function () {
    if (this.txs == null) return;
    if (this.txList == null) return;
    
    var n, nn;
    
    /*this.txs.sort(function (a, b) { return (b.blockheight - a.blockheight); });*/
    
    if (this.txList.tBodies)
    {
        var old_tbody = this.txList.tBodies[0];
        var new_tbody = document.createElement('tbody');
    
        for (n = 0; n < this.txs.length; n++) {
            var row = new_tbody.insertRow(n * 2);
            var cell;
            row.className = "txhdr";
    
            cell = row.insertCell(0);
            cell.className = "tx_hash";
            cell.innerHTML = '<span class="tx_expand" onclick="show_tx(\'' + this.txs[n].txid + '\'); if(this.innerHTML==\'+\'){ this.innerHTML=\'-\'; }else{ this.innerHTML=\'+\'; } ">+</span><a class="tx_lnk" onclick="SelectTx(\'' + this.txs[n].txid + '\'); return false;" href="' + site_base_url + '/tx/' + this.txs[n].txid + '">' + '#' + n + '</a>&nbsp;' + timeConverter(this.txs[n].blocktime);
    
            cell = row.insertCell(1);
            cell.className = "txmine";
            cell.innerHTML = 'block #' + this.txs[n].blockheight + '&nbsp;' + timeConverter(this.txs[n].blocktime);
    
    
            row = new_tbody.insertRow(n * 2 + 1);
    
            row.id = "tx_infos_" + this.txs[n].txid;
            row.className = "tx_infos";
    
            if (this.txs[n].isNull == true) {
                cell = row.insertCell(0);
                cell.className = "txins";
                cell.innerHTML = '#0 null <br/>';
                cell = row.insertCell(1);
                cell.className = "txouts";
                cell.innerHTML = '#0 null <br/>';
            }
            else if (this.txs[n].is_app_root == 1) {
                cell = row.insertCell(0);
                cell.className = "txins";
                cell.innerHTML = 'approot <br/>';
                cell = row.insertCell(1);
                cell.className = "txouts";
    
    
                cell.innerHTML = '#0 ' + this.txs[n].vout[0].value + ' ' + this.txs[n].dstaddr + '<br/>';
            }
            else {
                cell = row.insertCell(0);
                cell.className = "txins";
                if (this.txs[n].isCoinBase == false) {
                    var html = '';
    
                    for (nn = 0; nn < this.txs[n].vin.length; nn++) {
                        html += '#' + this.txs[n].vin[nn].n + '&nbsp' + ' <a href= "' + site_base_url + '/address/' + this.txs[n].vin[nn].addresses[0] + '" class="tx_address">' + this.txs[n].vin[nn].addresses[0] + '</a>' + '&nbsp' + this.txs[n].vin[nn].value / unit + '  <br/>';
                    }
                    cell.innerHTML = html;
                }
                else if ((this.txs[n].vin) && (this.txs[n].vin.length > 0)) {
                    cell.innerHTML = 'coin base' + this.txs[n].vin[0].coinbase;
                }
                cell = row.insertCell(1);
                cell.className = "txouts";
    
                html = '';
                if (this.txs[n].vout) {
                    for (nn = 0; nn < this.txs[n].vout.length; nn++) {
                        if (this.txs[n].vout[nn].isNull == true)
                            html += '#0 null <br/>';
                        else {
                            html += '#' + this.txs[n].vout[nn].n + '&nbsp' + ' <a href="' + site_base_url + '/address/' + this.txs[n].vout[nn].addresses[0] + '" class="tx_address">' + this.txs[n].vout[nn].addresses[0] + '</span> ' + '&nbsp' + this.txs[n].vout[nn].value / unit + ' <br/>';
                        }
                    }
                }
                cell.innerHTML = html;
            }
        }
        old_tbody.parentNode.replaceChild(new_tbody, old_tbody);
    }
    else {
        var new_html = '';
        for (n = 0; n < this.txs.length; n++) {
            new_html += get_tx_html(this.txs[n], n);
        }
        this.txList.innerHTML = new_html;
    }
}

BlockExplorer.prototype.update_filter_list = function () {
    var html = 'filters : <br/>';
    
    for (var i = 0; i < this.blk_filters.length; i++) {
        html += '<div><span onclick="MyBlocks.remove_filter(' + i + ');" style="color:red;">X</span>' + this.blk_filters[i] + '</div>';
    }
    this.blk_filts.innerHTML=html;
}

BlockExplorer.prototype. update_addr_balance = function ()
{
    var balance = this.addrRecv - this.addrSent;
    $("#Received").html(this.addrRecv / unit);
    $("#Sent").html(this.addrSent / unit);
    $("#Balance").html(balance / unit);
}



BlockExplorer.prototype.get_tx = function  (hash) {
    var self = this;
    api_call('tx', '/' + hash, function (tx) { self.update_tx(tx); });
}

BlockExplorer.prototype.remove_filter = function (i)
{
    if (i >= this.blk_filters.length) return;
    this.blk_filters.splice(i, 1);
    this.update_filter_list();
}

BlockExplorer.prototype.add_block_filter = function (key, op, val) {
    var new_filter = key + op + val;
    var found = 0;
    for (var i = 0; i < this.blk_filters.length; i++)
    {
        if (this.blk_filters[i].indexOf(key) == 0)
        {
            var fop = this.blk_filters[i].substr(key.length, 1);
            found = 1;
        
            if (fop == op) {
                this.blk_filters[i] = new_filter;
            }
            else {
                var fval = parseInt(this.blk_filters[i].substr(key.length + 1, 1));
                switch(fop)
                {
                    case '>':
                        switch (op) {
                            case '<': if (fval > val) this.blk_filters[i] = new_filter; else this.blk_filters.push(new_filter); break;
                            case '=': this.blk_filters[i] = new_filter; break;
                        }
                    break;
                    case '<':
                        switch (op) {
                           case '>': if (fval < val) this.blk_filters[i] = new_filter; else this.blk_filters.push(new_filter); break;
                           case '=': this.blk_filters[i] = new_filter; break;
                        }
                    break;
                    case '=': this.blk_filters[i] = new_filter; break;
                }
            }
        }
    }
    
    if (!found)
        this.blk_filters.push(new_filter);
    
    this.update_filter_list();
}

   

BlockExplorer.prototype.list_block_txs = function (hash) {
    var self = this;
    
    api_call('txs', '?block=' + hash + '&pageNum=' + this.tx_page_idx, function(data) {
        if (self.txs == null)
            self.txs = data.txs;
        else
            self.txs.push.apply(self.txs, data.txs);

        self.txs.sort(function (a, b) { return (b.blockheight - a.blockheight); });
    
        self.update_txs();
    
        if (self.txs.length < data.numtx)
        {
            $('#txloadmore').prop("disabled", false);
            $('#txloadmore').removeProp("disabled");
        }
        else
            $('#txloadmore').prop("disabled", true);
    
        $("#curtxs").html(self.txs.length);
        $("#totaltxs").html(data.numtx);
    });
}

BlockExplorer.prototype.list_txs = function (date) {
    var self = this;
    
    api_call('txs', "?BlockDate=" + date + '&pageNum=' + this.tx_page_idx, function(data) {
    
        if (data != null)
        {
            if (self.txs == null)
                self.txs = data.txs;
            else
                self.txs.push.apply(self.txs, data.txs);
    
            self.update_txs();
    
    
            if (self.txs.length < data.numtxs)
            {
                $('#txloadmore').prop("disabled", false);
                $('#txloadmore').removeProp("disabled");
            }
            else
                $('#txloadmore').prop("disabled", true);
    
            $("#curtxs").html(self.txs.length);
            $("#totaltxs").html(data.numtxs);
        }
        else {
            $('#txloadmore').prop("disabled", true);
            $("#curtxs").html('0');
            $("#totaltxs").html('0');
        }
    
    });
}

BlockExplorer.prototype.list_blocks = function  (date, lastBlock) {
    var self = this;
    var urlq = '';
    var first = 1;
    
    if (this.blkTable == null) return;
    
    if(date!=null)
    {
        urlq = 'BlockDate=' + date ;
        first = 0;
    }
    
    if (this.blk_page_idx > 0) {
        if (!first) urlq += '&';
        urlq += 'pageNum=' + this.blk_page_idx;
        first = 0;
    }
    
    
    if (lastBlock > 0)
    {
        if (!first) urlq += '&';
        urlq += 'height<' + (lastBlock+1);
        first = 0;
        this.lastLastBlock = lastBlock;
    }
    
    
    for (var i = 0; i < this.blk_filters.length; i++)
    {
        if (!first) urlq += '&';
        urlq += this.blk_filters[i];
        first = 0;
    }
    
    this.create_blk_tbl();
    
    api_call('blocks', '?' + urlq, function (data) {
    
        var mdate;
    
        if (data != null) {
            if (self.blocks == null)
                self.blocks = data.blocks;
            else
                self.blocks.push.apply(self.blocks, data.blocks);
    
            self.blocks.sort(function (a, b) { return (b.height - a.height); });
        }
    
        if (date != null) {
            
            self.CurrentTime = Math.round(new Date(date).getTime() / 1000);
            self.PrevTime    = self.CurrentTime - 24 * 3600;
            self.NextTime    = self.CurrentTime + 24 * 3600;
    
    
            mdate = dateConverter(self.CurrentTime);
            $("#blocklistdate").html(mdate);
    
            mdate = dateConverter(self.NextTime);
            $("#blocklistnext").html(mdate);
    
            mdate = dateConverter(self.PrevTime);
            $("#blocklistprev").html(mdate);
        }
        else {
            $("#blocklistdate").empty();
            $("#blocklistnext").empty();
            $("#blocklistprev").empty();
        }
    
        self.update_blocks();
    
        if ((self.blocks != null) && (data != null)) {
            $("#curblocks").html(self.blocks.length);
            $("#totalblocks").html(data.numblocks);
    
            if (data.lastblockidx > 0)
            {
                $('#searchmore').prop("disabled", false);
                $('#searchmore').removeProp("disabled");

                $('#cursearch').prop("disabled", false);
                $('#cursearch').removeProp("disabled");

                $('#cursearch').val(data.lastblockidx);
            }
            else
            {
                $('#cursearch').prop("disabled", true);
                $('#searchmore').prop("disabled", true);
                $('#cursearch').val('0');
            }
    
            if (self.blocks.length < data.numblocks)
            {
                $('#blkloadmore').prop("disabled", false);
                $('#blkloadmore').removeProp("disabled");
            }
            else
                $('#blkloadmore').prop("disabled", true);
        }
        else
        {
            $('#searchmore').prop("disabled", true);
            $('#cursearch').val('');
            $("#curblocks").html('0');
            $("#totalblocks").html('0');
            $('#blkloadmore').prop("disabled", true);
        }
        
    });
}


BlockExplorer.prototype.setCalDate = function (UNIX_timestamp) {
    var a = new Date(UNIX_timestamp * 1000);
    var months = ['01', '02', '03', '04', '05', '06', '07', '08', '09', '10', '11', '12'];
    var year = a.getFullYear();
    var month = months[a.getMonth()];
    var day;
    
    if (a.getDate() < 10)
        day = '0' + a.getDate();
    else
        day = a.getDate();
    
    $("#block_y").val(year);
    $("#block_m").val(month);
    $("#block_d").val(day);
    
    this.update_blocks_calendar(day,0);
}

BlockExplorer.prototype.get_lasttxs = function () {
    var self = this;
    
    api_call('txs', '', function (data) {
    
        self.TxpageNum = 0;
        self.addrPageNum = 0;
        self.tx_page_idx = 0;
        self.lastblock = data;
        self.txs = data.txs;
    
        self.update_txs();
    
        $('#txloadmore').prop("disabled",false);
        $('#txloadmore').removeProp("disabled");
        $("#curtxs").html(self.txs.length);
        $("#totaltxs").html('--');
    });
}

BlockExplorer.prototype.get_lastblock = function () {
    var self = this;
    
    api_call('block', '', function(block) {
        var tdate;
        var date;
        self.lastblock = block;
    
        if(document.getElementById('inline'))
            setCalDate(self.lastblock.time);
    
        tdate = dateConverter(self.lastblock.time);
        $("#blocklistdate").html(tdate);
    
        self.PrevTime = block.time - 24 * 3600;
        self.NextTime = block.time + 24 * 3600;
    
        date = dateConverter(self.NextTime);
        $("#blocklistnext").html(date);
    
        date = dateConverter(self.PrevTime);
        $("#blocklistprev").html(date);
    
        self.updateblock(block);
        self.blocks = null;
        self.txs = null;
        self.list_blocks(tdate);
    });
}



BlockExplorer.prototype.setlongpoll = function(in_url)
{
    var self = this;
    console.log("long poll " + site_base_url + in_url);

    $.ajax({
        url: site_base_url + in_url,
        type: "GET",
        dataType: "json",
        success: function () { self.get_lastblock(); self.setlongpoll(in_url); },
        error: function (err) { /*alert("Error");*/ }
    });


    
}

BlockExplorer.prototype.seteventsrc = function (in_url, handler) {
    var self = this;
    
    this.evtSource = new EventSource(site_base_url + in_url);

    this.evtSource.addEventListener("newblock", handler, false);

}

BlockExplorer.prototype.get_addr_balance = function ()
{
    var self = this;
    api_call('addrbalance/' + this.currentAddr, '', function (data) {

        self.addrRecv = data.recv;
        self.addrSent = data.sent;
        self.update_addr_balance();
    });
}

BlockExplorer.prototype.list_addr_txs = function  () {
    var self = this;
    
    $('#address').html(this.currentAddr);
    
    api_call('txs', '?address=' + this.currentAddr + '&pageNum=' + this.tx_page_idx, function(data) {
    
        if (self.txs == null)
            self.txs = data.txs;
        else
            self.txs.push.apply(self.txs, data.txs);
    
        /* self.txs.sort( function (a, b) { return (b.blockheight - a.blocktime); }); */
    
        self.update_addr_txs();
    
        $("#Transactions").html(data.numtx);
        $("#currentaddrtx").html(self.txs.length);
        $("#totaladdrtx").html(data.numtx);
    
        if (self.txs.length < data.numtx)
        {
            $('#loadmore').prop("disabled", false);
            $('#loadmore').removeProp("disabled");
        }
        else
            $('#loadmore').prop("disabled", true);
    });
}

BlockExplorer.prototype.new_blocks = function  () {
    api_call('blocks', "?SinceBlock=" + this.blocks[0].hash, function(data) {
        var updt_blocks = data.blocks;
        $("#newblocks").html(updt_blocks.length);
    });
}

BlockExplorer.prototype.selectAddress = function  (saddr) {
    this.tx_page_idx = 0;
    this.txs = null;
    this.currentAddr = saddr.toString();
    $("#address_list").html(this.currentAddr);
    this.list_addr_txs();
}

BlockExplorer.prototype.selectBlock = function (hash) {
    var self = this;
    
    if (this.selectedhash != null) {
        $('#block_' + this.selectedhash).removeClass("selected");
        this.selectedhash = null;
    }
    
    api_call('block', '/' + hash, function (blk_data) {
        var block = blk_data;
        self.blk_page_idx = 0;
        self.selectedhash = block.hash;
    
        self.blocks = new Array(block);
        self.updateblock(block);
        self.update_blocks();
                   
    
        $('#blkloadmore').prop('disabled', true);
        $("#curblocks").html("1");
        $("#totalblocks").html("1");
        $('#block_' + self.selectedhash).addClass("selected");
        $('#search_bar').val(self.selectedhash);
    });
}

BlockExplorer.prototype.SelectTx = function (hash) {
    var self = this;

    api_call('tx', '/' + hash, function(tx_data) {
        var tx = tx_data;
        self.txs = new Array(tx);
        self.update_txs();

        $('#txloadmore').prop("disabled", true);
        $("#curtxs").html("1");
        $("#totaltxs").html("1");
        $('#search_bar').val(tx.txid);

        if (self.selectedhash != null) {
            $('#block_' + self.selectedhash).removeClass("selected");
            self.selectedhash = null;
        }

        api_call('block', '/' + tx.blockhash, function(blk_data) {
                var block = blk_data;
                self.blocks = new Array(block);
                self.updateblock(block);
                self.update_blocks();

                self.selectedhash = block.hash;

                $('#blkloadmore').prop('disabled', true);
                $("#curblocks").html("1");
                $("#totalblocks").html("1");
                $('#block_' + self.selectedhash).addClass("selected");
        });
    });
}


BlockExplorer.prototype.selectBlockTxs = function (hash) {
    var self = this;
    
    $('#imp_hash').val(hash);
    
    if (this.selectedhash != null) {
        $('#block_' + this.selectedhash).removeClass("selected");
        this.selectedhash = null;
    }
    
    api_call('block', '/' + hash, function(blk_data) {
        var block = blk_data;
        self.blocks = new Array(block);
        self.updateblock(block);
        self.update_blocks();
    
        self.tx_page_idx = 0;
        self.blk_page_idx = 0;
        self.txs = null;
        self.selectedhash = block.hash;
    
        self.list_block_txs(self.selectedhash);
    
        $('#blkloadmore').prop('disabled', true);
        $("#curblocks").html("1");
        $("#totalblocks").html("1");
        $('#search_bar').val(self.selectedhash);
        $('#block_' + hash).addClass("selected");
    });
}

BlockExplorer.prototype.create_block_infos = function (root_element) {
    var self = this;
    var rows = [{ label: "height :", id: "height" },
                { label: "time :", id: "time" }, 
                { label: "hash :", id: "hash" },
                { label: "prev block :", id: "previousblockhash" },
                { label: "next block :", id: "nextblockhash" },
                { label: "confirmations :", id: "confirmations" },
                { label: "difficulty :", id: "difficulty" },
                { label: "merkleroot :", id: "merkleroot" },
                { label: "nonce :", id: "nonce" },
                { label: "bits :", id: "bits" },
                { label: "difficulty hash :", id: "diffhash" },
                { label: "proofhash :", id: "proofhash" },
                { label: "size :", id: "size" },
                { label: "stakemodifier2 :", id: "stakemodifier2" },
                { label: "version :", id: "version" },
                { label: "txs :", id: "txs" }];
    var container, row, cell;
    
    container = document.createElement('div');
    container.className = "container";
    container.setAttribute('style', 'font-size:0.8em;')
    
    for (var n = 0; n < rows.length; n++)
    {
        row = document.createElement('div');
        row.className = "row";

        cell = document.createElement('div');
        cell.className  = "col-sm-2 lbl";
        cell.innerHTML = rows[n].label;
        row.appendChild(cell);

        cell = document.createElement('div');
        cell.className = "col-md-8";
        cell.id = rows[n].id;
        row.appendChild(cell);

        container.appendChild(row);
    }
    
    document.getElementById(root_element).appendChild(container);
    
    cell            = document.getElementById('previousblockhash');
    cell.className = "col-md-8 val ihash";
    cell.addEventListener("click", function () { self.selectBlockTxs(this.innerHTML); });
    
    cell            = document.getElementById('nextblockhash');
    cell.className = "col-md-8 val ihash";
    cell.addEventListener("click", function () { self.selectBlockTxs(this.innerHTML); });
}

BlockExplorer.prototype.create_day_nav = function (root_element,currentTime) {
    var self = this;;
    var container, span, row, col, h3;
    
    this.CurrentTime = currentTime;
    this.PrevTime    = this.CurrentTime - 24 * 3600;
    this.NextTime    = this.CurrentTime + 24 * 3600;
    
    container   = document.createElement('div');
    row         = document.createElement('div');
    
    container.className = 'container';
    row.className = 'row';
    
    col = document.createElement('div');
    h3 = document.createElement('h3');
    span = document.createElement('span');
    
    col.className = 'col-md';
    h3.innerHTML = 'prev';
    span.id = 'blocklistprev';
    span.innerHTML = dateConverter(this.PrevTime);
         
    span.addEventListener("click", function () {
    
        self.blocks = null;
        self.txs = null;
        self.blk_page_idx = 0;
        self.tx_page_idx = 0;
    
        $('#datepicker').datepicker('setDate', dateConverter(self.PrevTime));
    
        self.list_blocks(dateConverter(self.PrevTime), 0);
        self.list_txs   (dateConverter(self.PrevTime));
    
    });
    
    col.appendChild(h3);
    col.appendChild(span);
    row.appendChild(col);
    
    
    col = document.createElement('div');
    h3 = document.createElement('h3');
    span = document.createElement('span');
    
    col.className = 'col-md';
    h3.innerHTML = 'today';
    span.id = 'blocklistdate';
    span.innerHTML = dateConverter(this.CurrentTime);
    
    
    span.addEventListener("click", function () {
        self.blocks = null;
        self.txs = null;
        self.blk_page_idx = 0;
        self.tx_page_idx = 0;
    
        $('#datepicker').datepicker('setDate', dateConverter(self.CurrentTime));
    
        self.list_blocks(dateConverter(self.CurrentTime), 0);
        self.list_txs   (dateConverter(self.CurrentTime));
    
    });
    
    col.appendChild(h3);
    col.appendChild(span);
    row.appendChild(col);
    
    col = document.createElement('div');
    h3 = document.createElement('h3');
    span = document.createElement('span');
    
    col.className = 'col-md';
    h3.innerHTML = 'next';
    span.id = 'blocklistnext';
    span.innerHTML = dateConverter(this.NextTime);
    
    span.addEventListener("click", function () {
        self.blocks = null;
        self.txs = null;
        self.blk_page_idx = 0;
        self.tx_page_idx = 0;
    
        $('#datepicker').datepicker('setDate', dateConverter(self.NextTime));
    
        self.list_blocks(dateConverter(self.NextTime), 0);
        self.list_txs   (dateConverter(self.NextTime));
    });
    
    col.appendChild(h3);
    col.appendChild(span);
    row.appendChild(col);
    container.appendChild(row);
    
    
    document.getElementById(root_element).appendChild(container);
}

BlockExplorer.prototype.create_tx_panel = function (root_element, txid) {
    var self = this;
    var panel, panel_hdr, panel_body, h3, i, span, table, row, cell,div, input;
    var txinfos = [{ label: 'size', id: 'size' }, { label: 'Received Time', id: 'txtime' }, { label: 'Mined Time :', id: 'blocktime' }, { label: 'Included in Block', id: 'txblock' }]
    
    div = document.createElement('div');
    span = document.createElement('span');
    
    
    div.setAttribute('style', 'text-align:center;vertical-align:bottom;padding-bottom:8px;');
    
    span.innerHTML = 'enter a block or tx hash.';
    div.appendChild(span);
    
    input       = document.createElement('input');
    input.type  = "text";
    input.id    = 'imp_tx';
    input.name  = 'imp_tx';
    input.size = 64;
    
    if (txid.length == 64)
    {
        input.value = txid;
        this.get_tx(txid);
    }
    
    div.appendChild(input);
    
    input = document.createElement('input');
    input.type = "button";
    input.value = 'search';
    
    input.addEventListener("click", function () { self.get_tx($('#imp_tx').val()); });
    
    div.appendChild(input);
    
    document.getElementById(root_element).appendChild(div);
    
    panel = document.createElement('div');
    panel_hdr = document.createElement('div');
    panel_body = document.createElement('div');
    h3 = document.createElement('h3');
    i = document.createElement('i');
    span = document.createElement('span');
    
    panel.className = 'card';
    panel_hdr.className = 'card-header';
    panel_body.className = 'card-body';
    h3.className = '';
    i.className = 'fa fa-tasks';
    i.innerHTML = 'transaction&nbsp;';
    span.id = 'txhash';
    
    h3.appendChild(i);
    h3.appendChild(span);
    panel_hdr.appendChild(h3);
    
    table = document.createElement('table');
    
    for (var n = 0; n < txinfos.length; n++) {
        row = table.insertRow(n);
    
        cell = row.insertCell(0);
        cell.className = "lbl";
        cell.innerHTML = txinfos[n].label;

        cell = row.insertCell(1);
        cell.className = "val";
        cell.id = txinfos[n].id;
    }
    
    row = table.insertRow(txinfos.length);
    cell            = row.insertCell(0);
    cell.id         = 'coinbaselbl';
    cell.innerHTML  = 'Coinbase';
    
    cell            = row.insertCell(1);
    cell.id         = 'coinbase';
    
    panel_hdr.appendChild(table);
    panel.appendChild(panel_hdr);
    
    this.txList = document.createElement('div');
    this.txList.id = 'tx_list';
    
    panel_body.appendChild(this.txList);
    panel.appendChild(panel_body);
    
    document.getElementById(root_element).appendChild(panel);
}

BlockExplorer.prototype.create_addr_panel = function (root_element,addr) {
    var self = this;
    var n;
    var txinfos = [{ label: 'Total Received :', id: 'Received' }, { label: 'Total Sent :', id: 'Sent' }, { label: 'Final Balance :', id: 'Balance' }, { label: 'No. Transactions :', id: 'Transactions' }]
    var root, panel, panel_hdr, panel_body, panel_footer, input, h3, i, span, row, container, col;
    
    this.currentAddr = addr;
    
    root=document.getElementById(root_element);
    
    input = document.createElement('input');
    input.type = 'text';
    input.id = 'imp_addr';
    input.name = 'imp_addr';
    input.size = 64;
    input.value = this.currentAddr;
    
    root.appendChild(input);
    
    input = document.createElement('input');
    input.type = 'button';
    input.value = 'search';
    
    input.addEventListener("click", function () {
    
        self.tx_page_idx = 0;
        self.txs         = null;
        self.currentAddr = $('#imp_addr').val();
        
        self.list_addr_txs();
    });
    
    root.appendChild(input);
    
    
    panel                   = document.createElement('div');
    panel_hdr               = document.createElement('div');
    panel_body              = document.createElement('div');
    container               = document.createElement('div');
    h3                      = document.createElement('h3');
    i                       = document.createElement('i');
    span                    = document.createElement('span');
    
    panel.className         = 'card';
    panel_hdr.className     = 'card-header';
    panel_body.className    = 'card-body';
    container.className     = 'container';
    h3.className            = '';
    i.className             = 'fa fa- tasks';
    i.innerHTML             = 'address&nbsp;';
    span.id                 = 'address';
    
    h3.appendChild          (i);
    h3.appendChild          (span);
    panel_hdr.appendChild   (h3);
    panel.appendChild       (panel_hdr);
    
    for (n = 0; n < txinfos.length; n++)
    {
        row = document.createElement('div');
        row.className = "row";
    
        col = document.createElement('div');
        col.className = 'col-md-2';
        col.innerHTML = txinfos[n].label;
    
        row.appendChild (col);
    
        col = document.createElement('div');
        col.className = 'col-md-4 justify-content-end';
        col.id = txinfos[n].id;
    
        row.appendChild (col);
    
        container.appendChild(row);
    }
    panel_body.appendChild(container);
    panel.appendChild(panel_body);
    root.appendChild(panel);

    root.appendChild(document.createElement('hr'));
    
    
    panel = document.createElement('div');
    panel_hdr = document.createElement('div');
    panel_body = document.createElement('div');
    panel_footer = document.createElement('div');
    h3 = document.createElement('h3');
    i = document.createElement('i');
    
    panel.className = 'card';
    panel_hdr.className = 'card-header';
    panel_body.className = 'card-body';
    panel_footer.className = 'card-footer';
    h3.className = '';
    i.className = 'fa fa- tasks';
    i.innerHTML = 'Transactions';
    
    h3.appendChild(i);
    panel_hdr.appendChild(h3);
    panel.appendChild(panel_hdr);
    
    span = document.createElement('span');
    span.id = 'currentaddrtx';
    panel_body.appendChild(span);
    
    span = document.createElement('span');
    span.innerHTML = '&nbsp;/&nbsp;';
    panel_body.appendChild(span);
    
    span = document.createElement('span');
    span.id = 'totaladdrtx';
    panel_body.appendChild(span);
    
    input = document.createElement('input');
    input.type  = 'button';
    input.id    = 'loadmore';
    input.value = 'load 10 more';
    
    input.addEventListener("click", function () {
        self.tx_page_idx++;
        self.list_addr_txs();
    });
    
    panel_footer.appendChild(input);
    
    this.txList = document.createElement('div');
    this.txList.id = 'tx_list';
    
    panel_body.appendChild  (this.txList);
    panel.appendChild(panel_body);
    panel.appendChild(panel_footer);
    root.appendChild        (panel);
}



BlockExplorer.prototype.create_block_list = function (root_element)
{
    var self = this;
    var filters = [{ val: "height", label: "height" }, { val: "time", label: "time" }, { val: "reward", label: "reward" }, { val: "tx*", label: "ntx" }, { val: "size", label: "size" }];
    var filter_ops = [{ val: ">", label: ">" }, { val: "<", label: "<" }, { val: "=", label: "=" }];
    var container, span, row, col, inrow, incol, panel, panel_hdr, panel_body, panel_footer, h3, i, select, input;
    var n;

    this.blkTable = null;
    
    container = document.createElement('div');
    row = document.createElement('div');
    col = document.createElement('div');
    
    
    panel = document.createElement('div');
    panel_hdr = document.createElement('div');
    panel_body = document.createElement('div');
    this.blk_filts = document.createElement('div');
    h3 = document.createElement('h3');
    i = document.createElement('i');
    
    
    container.className = 'container';
    row.className = 'row';
    col.className = 'col-md';
    panel.className = 'card';
    panel_hdr.className = 'card-header';
    h3.className = 'panel-title';
    i.className = 'fa fa- tasks';
    this.blk_filts.id = 'blk_filter_list';
    i.innerHTML = 'Block';
    
    h3.appendChild(i);
    panel_hdr.appendChild(h3);
    panel_hdr.appendChild(this.blk_filts);
    panel.appendChild(panel_hdr);
    
    
    inrow                   = document.createElement('div');
    incol                   = document.createElement('div');
    this.blk_filter_key     = document.createElement('select');
    
    panel_body.className    = 'card-body';
    inrow.className         = 'row';
    incol.className         = 'col-md';
    this.blk_filter_key.id = 'blk_filter_key';
    this.blk_filter_key.className = 'browser-default';
    
    for (n = 0; n < filters.length; n++)
    {
        var opt = document.createElement('option');
        opt.text = filters[n].label;
        opt.value = filters[n].val;
        this.blk_filter_key.add(opt);
    }
    
    incol.appendChild(this.blk_filter_key);
    inrow.appendChild(incol);
    
    incol               = document.createElement('div');
    incol.className     = 'col-md';
    
    this.blk_filter_op      = document.createElement('select');
    this.blk_filter_op.id = 'blk_filter_op';
    this.blk_filter_op.className = 'browser-default';
    
    for (n = 0; n < filter_ops.length; n++) {
        var opt = document.createElement('option');
        opt.text = filter_ops[n].label;
        opt.value = filter_ops[n].val;
        this.blk_filter_op.add(opt);
    }
    
    incol.appendChild(this.blk_filter_op);
    inrow.appendChild(incol);
    
    
    incol           = document.createElement('div');
    incol.className = 'col-md';
    
    this.blk_filter_val         = document.createElement('input');
    this.blk_filter_val.type    = "text";
    this.blk_filter_val.id      = 'blk_filter_val';
    
    incol.appendChild   (this.blk_filter_val);
    inrow.appendChild   (incol);
    
    
    incol           = document.createElement('div');
    input           = document.createElement('input');
    input.type      = 'button';
    input.value     = 'add filter';
    
    
    input.addEventListener("click", function ()
    {
        self.add_block_filter(self.blk_filter_key.value, self.blk_filter_op.value, self.blk_filter_val.value );
        self.blk_page_idx = 0;
        self.blocks = null;
        self.list_blocks(null);
    
    });
    
    
    incol.appendChild(input);
    inrow.appendChild(incol);
    panel_body.appendChild(inrow);
    
    
    this.create_blk_tbl();
        
    panel_body.appendChild(this.blkTable);
    panel.appendChild(panel_body);
    
    panel_footer = document.createElement('div');
    panel_footer.className = 'panel-footer';
    
    
    input = document.createElement('input');
    input.id = 'blkloadmore';
    input.type = 'button';
    input.value = 'load 10 more';
    input.addEventListener("click", function () {
    
        var mdate;
    
        if (self.blk_filters.length == 0)
            mdate = dateConverter(self.CurrentTime);
        else
            mdate = null;
    
        self.blk_page_idx++;
    
        self.list_blocks(mdate, this.lastLastBlock);
    
    });
    
    panel_footer.appendChild(input);
    
    span = document.createElement('span');
    span.id = 'curblocks';
    panel_footer.appendChild(span);
    
    span = document.createElement('span');
    span.innerHTML = '&nbsp;/&nbsp;';
    panel_footer.appendChild(span);
    
    span = document.createElement('span');
    span.id = 'totalblocks';
    panel_footer.appendChild(span);
    
    input = document.createElement('input');
    input.id = 'searchmore';
    input.type = 'button';
    input.value = 'search more';
    
    input.addEventListener("click", function () {
        self.blk_page_idx = parseInt(self.cursearch.value);
        self.list_blocks(null, this.lastLastBlock);
    
    });
    
    panel_footer.appendChild(input);
    span                    = document.createElement('span');
    span.innerHTML          = 'from block #';
    
    this.cursearch          = document.createElement('input');
    this.cursearch.id       = 'cursearch';
    this.cursearch.type     = 'text';
    this.cursearch.value    = '0';
    this.cursearch.setAttribute       ('disabled', 'disabled');
    
    panel_footer.appendChild(span);
    panel_footer.appendChild(this.cursearch);
    
    panel.appendChild(panel_footer);
    col.appendChild(panel);
    row.appendChild(col);
    
    col = document.createElement('div');
    panel = document.createElement('div');
    panel_hdr = document.createElement('div');
    panel_body = document.createElement('div');
    panel_footer = document.createElement('div');
    this.txList = document.createElement('div');
    
    h3 = document.createElement('h3');
    i = document.createElement('i');
    
    col.className = 'col-md';
    panel.className = 'card';
    panel_hdr.className = 'card-header';
    panel_body.className = 'card-body';
    panel_footer.className = 'card-footer';
    h3.className = 'panel-title';
    i.className = 'fa fa- tasks';
    i.innerHTML = 'Transactions';
    this.txList.id = 'tx_list';
    
    h3.appendChild(i);
    panel_hdr.appendChild(h3);
    panel.appendChild(panel_hdr);
    
    panel_body.appendChild(this.txList);
    panel.appendChild(panel_body);
    
    input = document.createElement('input');
    input.id = 'txloadmore';
    input.type = 'button';
    input.value = 'load 10 more';
    
    input.addEventListener("click", function () {
    
        self.tx_page_idx++;
    
        if(MyBlocks.selectedhash==null)
            self.list_txs(dateConverter(self.CurrentTime));
        else
            self.list_block_txs(MyBlocks.selectedhash);
    
    });
    
    panel_footer.appendChild(input);
    
    span = document.createElement('span');
    span.id = 'curtxs';
    panel_footer.appendChild(span);
    
    
    span = document.createElement('span');
    span.innerHTML = '&nbsp;/&nbsp;';
    panel_footer.appendChild(span);
    
    span = document.createElement('span');
    span.id = 'totaltxs';
    panel_footer.appendChild(span);
    
    panel.appendChild(panel_footer);
    
    col.appendChild(panel);
    row.appendChild(col);
    container.appendChild(row);
    
    document.getElementById(root_element).appendChild(container);
}




var MyBlocks = null;