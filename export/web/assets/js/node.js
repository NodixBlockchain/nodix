
function Node () {
    this.mods = null;
    this.handlers = null;
    this.modules_definitions = {};
}


Node.prototype.make_node_html= function (name, node) {
    var html = '<div class="container" style="border-bottom:2px #000 dashed;" >';
    
    html += make_var_html("user agent", node.user_agent,'');

    if (typeof node.p2p_addr != 'undefined') {
        html += make_var_html("address", node.p2p_addr.addr, '');
        html += make_var_html("port", node.p2p_addr.port, '');
    }

    html += make_var_html("version", node.version, '');
    html += make_var_html("height", node.block_height, '');

    if (typeof node.node_pubkey != 'undefined') {
        html += make_var_html("public key", node.node_pubkey, 'hash-lnk');
    }

    html += make_var_html("ping", node.ping_delay + ' ms','');

    if (typeof (node.last_block) != 'undefined')
    {
        html += make_var_html("last block", node.last_block, 'hash-lnk');
    }

    html += '</div>';

    $('#' + name).append(html);
    
}
    

Node.prototype.make_modules_html = function (name, modules) {
    var self = this;
    var row,container;

    container = document.createElement('div');
    container.className = 'container flex-center text-center';

    row = document.createElement('div');
    row.className = 'row ';

    $('#' + name).empty();
    for (var i = 0; i < modules.length; i++) {

        var button,icon,span,col,br;

        if (typeof modules[i].module != 'undefined')
            var mDef = modules[i].module;
        else
            var mDef = modules[i];

        col = document.createElement('div');
        button = document.createElement('button');
        icon = document.createElement('i');
        span = document.createElement('span');
        br = document.createElement('br');

        col.className = 'col-md';

        button.className = 'btn btn-primary btn-rounded waves-effect';
        button.id = 'heading_' + mDef.name;
        button.type = 'button';
        button.setAttribute('data-toggle', "collapse");
        button.setAttribute('data-target', '#mod_infos_' + mDef.name);
        button.setAttribute('aria-expanded', false);
        button.setAttribute('aria-controls','mod_infos_' + mDef.name);
        
        icon.className = "fa fa-cog  fa-2x";
        span.innerHTML = mDef.name;

        button.appendChild(icon);
        button.appendChild(br);
        button.appendChild(span);

        col.appendChild(button);
        row.appendChild(col);
    }

    container.appendChild(row);

    $('#' + name).append(container);

    //flex-center text-center

    for (var i = 0; i < modules.length; i++) {

        var html = '';

        if (typeof modules[i].module != 'undefined')
            var mDef = modules[i].module;
        else
            var mDef = modules[i];

        html += '<div class="collapse" role="tabpanel" aria-labelledby="heading_' + mDef.name + '" data-parent="#'+name+'" id="mod_infos_' + mDef.name + '" >';
        html += '<div class="mt-3">';
        html += '<div><span>file :</span><span>' + mDef.file + '</span></div>';
        html += '<div><span>size :</span><span>' + mDef.size + '</span>&nbsp;bytes</div>';
        html += '<h4>methods</h4>';
        html += '<ul id="mod_procs_' + mDef.name + '" style="padding-left:12px; list-style: square inside url(/assets/img/proc.gif);"  ;>'
        for (var n = 0; n < mDef.exports.length; n++) {
            html += '<li  method="' + mDef.exports[n] + '">'
            if (modules[i].type == 'cgi')
                html += '<a onclick="click_cgi_method(MyNode.mods[' + i + '],\'' + mDef.exports[n] + '\'); return false;" href="' + modules[i].base + mDef.exports[n] + '" >' + mDef.exports[n] + '</a>';
            else if (modules[i].type == 'rpc')
                html += '<a onclick="click_rpc_method(MyNode.mods[' + i + '],\'' + mDef.exports[n] + '\'); " href="#api_div" >' + mDef.exports[n] + '</a>';
            else
                html += '<a >' + mDef.exports[n] + '</a>';

            html += '<span  class="args"></span>';
            html += '<p  class="desc"></p>';
            html += '</li>';
        }
        html += '</ul>';
        html += '</div></div>';
        $('#' + name).append(html);
    }
}


Node.prototype.get_node_lag = function (node) {
    var n;
    var height = 0;
    for (n = 0; n < node.peer_nodes.length; n++) {
        height = Math.max(height, node.peer_nodes[n].block_height + 1);
    }
    var now = Math.floor(new Date().getTime() / 1000);
    var diff = now - node.last_block_time;
    var msec = diff;
    var dd = Math.floor(msec / (60 * 60 * 24));
    msec -= dd * 60 * 60 * 24;
    var hh = Math.floor(msec / (60 * 60));

    $('#node_time_lag').html(dd + ' days, ' + hh + 'hours');

    if (node.block_height <= height)
        $('#node_block_lag').html(height - node.block_height + '&nbsp; blocks behind');
    else
        $('#node_block_lag').html(node.block_height - height + '&nbsp; blocks ahead');

}

Node.prototype.update_mempool_txs = function (txs, tbl_name) {
        var n;

        txs.sort(function (a, b) { return (b.time - a.time); });

        if (document.getElementById(tbl_name).tBodies) {
            var old_tbody = document.getElementById(tbl_name).tBodies[0];
            var new_tbody = document.createElement('tbody');

            for (n = 0; n < txs.length; n++) {
                var row = new_tbody.insertRow(n * 2);
                var cell;
                row.className = "txhdr";

                cell = row.insertCell(0);
                cell.className = "tx_hash";
                cell.innerHTML = '<span class="tx_expand" onclick="show_tx(\'' + txs[n].txid + '\'); if(this.innerHTML==\'+\'){ this.innerHTML=\'-\'; }else{ this.innerHTML=\'+\'; } ">+</span><a class="tx_lnk" onclick="SelectTx(\'' + txs[n].txid + '\'); return false;" href="' + site_base_url + '/tx/' + txs[n].txid + '">' + '#' + n + '</a>&nbsp;' + timeConverter(txs[n].time);

                row = new_tbody.insertRow(n * 2 + 1);

                row.id = "tx_infos_" + txs[n].txid;
                row.className = "tx_infos";


                if (txs[n].is_app_root == 1) {
                    cell = row.insertCell(0);
                    cell.className = "txins";
                    cell.innerHTML = 'approot <br/>';
                    cell = row.insertCell(1);
                    cell.className = "txouts";
                    cell.innerHTML = '#0 ' + txs[n].vout[0].value + ' ' + txs[n].dstaddr + '<br/>';
                }
                else {
                    cell = row.insertCell(0);
                    cell.className = "txins";

                    var nins, nouts;
                    var html = '';

                    nins = txs[n].vin.length;
                    for (nn = 0; nn < nins; nn++) {
                        if (txs[n].vin[nn].srcapp) {
                            new_html += 'App&nbsp; : ' + vin[nn].srcapp;

                            if (txs[n].appChildOf) new_html += '&nbsp; child';
                            if (txs[n].app_item == 1) new_html += '&nbsp; type';
                            if (txs[n].app_item == 2) new_html += '&nbsp; obj';
                            if (txs[n].app_item == 3) new_html += '&nbsp; file';

                        }
                        else {
                            var hh = ' <a href= "' + site_base_url + '/address/' + txs[n].vin[nn].dstaddr + '" class="tx_address">' + txs[n].vin[nn].dstaddr + '</a>';
                            html += '#' + nn + '&nbsp' + hh + '&nbsp' + txs[n].vin[nn].value / unit + '  <br/>';
                        }
                    }
                    cell.innerHTML = html;

                    cell = row.insertCell(1);
                    cell.className = "txouts";

                    html = '';
                    if (txs[n].vout) {
                        nouts = txs[n].vout.length;
                        for (nn = 0; nn < nouts; nn++) {
                            var hh = ' <a href="' + site_base_url + '/address/' + txs[n].vout[nn].dstaddr + '" class="tx_address">' + txs[n].vout[nn].dstaddr + '</span> ';
                            html += '#' + nn + '&nbsp' + hh + '&nbsp' + txs[n].vout[nn].value / unit + ' <br/>';
                        }
                    }
                    cell.innerHTML = html;
                }
            }
            old_tbody.parentNode.replaceChild(new_tbody, old_tbody);
        }
        else {
            var new_html = '';
            $('#' + tbl_name).empty();
            for (n = 0; n < txs.length; n++) {
                new_html += get_tx_html(txs[n], n);
            }
            $('#' + tbl_name).html(new_html);
        }
}



Node.prototype.fill_module_def_html = function (def) {
    var self = this;

    this.modules_definitions[def.name] = def;

    $('#mod_infos_' + def.name).prepend('<h3>' + def.desc + '</h3>');
    $('#mod_procs_' + def.name + ' li').each(function (index) {
        var proc = self.find_proc_name(def, $(this).attr('method'));
        if (proc != null) {

            $(this).find('.desc').html(proc.desc);
            var args = '(';
            if (typeof proc.params != 'undefined') {
                var first = 1;
                for (var i = 0; i < proc.params.length; i++) {
                    if (!first)
                        args += ',&nbsp;';

                    args += proc.params[i].name;
                    args += '<span style="font-size:8px;">' + proc.params[i].desc + '</span>';
                    first = 0;
                }
            }
            args += ')';
            $(this).find('.args').html(args);
        }
    })
}

    
Node.prototype.make_scripts_html = function (name, script_list) {
    var scripts = script_list;
    var html = '';
    $('#' + name).empty();

    html = '<div style="padding-left:8px;" class="accordion" id="service_scripts">';
    
    for (var i = 0; i < scripts.length; i++) {
       
        html += '<section>'
        html += '<div>';
        html += '<h2 id="service_script_header_' + i + '" data-toggle="collapse" data-target="#service_script_container_' + i + '" aria-expanded="false" aria-controls="service_script_container_' + i + '"  class="service_script_hdr"><strong>' + scripts[i].file.toString() + '</strong></h2>';
        
        html += '<div id="service_script_container_' + i + '" role="tabpanel" aria-labelledby = "service_script_header_' + i + '" data-parent="#service_scripts" class="service_script_container collapse" >';
        html += '<div id="service_script_' + i + '" style="padding-left:18px;" class="accordion" >';


        for (var scriptk in scripts[i]) {
            var script_var = scripts[i][scriptk];
            if (script_var == null) continue;

            if (script_var.length == 0) continue;

            html += '<div class="row script_var_row">';

            var value_html = '';
          
            if ((typeof script_var == 'object') || (typeof script_var == 'array') || (script_var.length < 48)) {
                if (typeof script_var == 'object') {

                    var objkeys = Object.keys(script_var);

                    for (var ne = 0; ne < objkeys.length; ne++) {

                        if ((typeof script_var[objkeys[ne]] == 'object') || (typeof script_var[objkeys[ne]] == 'array'))
                            value_html += objkeys[ne] + ': object' + '<br/>';
                        else
                            value_html += objkeys[ne] + ':' + script_var[objkeys[ne]].toString() + '<br/>';
                    }
                }
                else if (typeof script_var == 'array')
                {
                    for (var ne = 0; ne < script_var.length; ne++) {
                        {
                            if ((typeof script_var[ne] == 'object') || (typeof script_var[ne] == 'array'))
                                value_html += ne + ': object' + '<br/>';
                            else
                                value_html += ne + ':' + script_var[ne].toString() + '<br/>';
                        }
                    }
                }
                else
                {
                    value_html += script_var.toString();
                }

                html += '<div class="col-md-3"><label><h3>' + scriptk + '</h3></label></div>';
                html += '<div class="col script_var" >' + value_html + '</div>';
            }
            else if ((typeof script_var == 'integer') || (typeof script_var == 'number')) {
                value_html = script_var.toString();

                html += '<div class="col-md-3"><label><h3>' + scriptk + '</h3></label></div>';
                html += '<div class="col script_var" >' + value_html + '</div>';
            }
            else
            {
                var str = '';
                var script_id = i + '_' + scriptk;

                if (typeof script_var == 'string')
                {
                    if ( ((script_var.length == 64)||(script_var.length == 66)) && (isHexStr(script_var)))
                    {
                        value_html = script_var.toString();

                        html += '<div class="col-md-3"><label><h3>' + scriptk + '</h3></label></div>';
                        html += '<div class="col script_var"  id="var_script_' + script_id + '" >' + value_html + '</div>';
                    }
                    else
                    {
                        var page_url;

                        value_html = script_var.replace(/<br\/>/g, '\n');
                        value_html = value_html.replace(/</g, '&lt;');
                        value_html = value_html.replace(/>/g, '&gt;');
                        value_html = value_html.replace(/(?:\r\n|\r|\n)/g, '<br/>');


                        page_url = scripts[i].script_url + '/' + scriptk;
                            
    
                        html += '<div class="col-md-3">';
                        html += '<h3 id= "script_hdr_' + script_id + '"  data-toggle="collapse" data-target="#var_script_' + script_id + '" aria-expanded="false" aria-controls="var_script_' + script_id + '"  script_key="' + scriptk + '" script_id="' + i + '" class="script_proc_hdr">' + scriptk + '</h3></div><div class="col-md-1"><a target="_blank" class="script-page-link" href="' + page_url + '"><i class="fa fa-link"></i></a></div>';
                        html += '</div>';
                        html += '<div class="row">';
                        html += '<div id = "var_script_' + script_id + '" role="tabpanel" aria-labelledby = "script_hdr_' + script_id + '" data-parent="#service_script_container_' + i + '"  class="col script_proc collapse" >' + value_html + '</div>';

                    }
                }
            }

            html += '</div>';
        }
        html += '</div>';
        html += '</section>';

       
    }
    $('#' + name).append(html);
}


Node.prototype.make_handlers_html = function (name, handlers_list) {
    $('#' + name).empty();
    this.handlers = handlers_list;
    var html = '';
    html = '<div>'
    html += '<section>'
    for (var handlerk in this.handlers) {
        var script_var = this.handlers[handlerk];
        var str = script_var.replace(/(?:\r\n|\r|\n)/g, '<br />');
        html += '<div>';
        html += '<label  ><h3 onclick=" $(\'#msg_' + handlerk + '\').slideToggle();">' + handlerk + '(node,payload)</h3></label>';
        html += '<div id="msg_' + handlerk + '" class="script_proc">' + str + '</div>';
        html += '</div>'
    }
    html += '</section>';
    html += '</div>'
    $('#' + name).append(html);

}

Node.prototype.find_proc_name = function (def, proc_name) {
    if (typeof def.methods != 'undefined') {
        for (var n = 0; n < def.methods.length; n++) {
            if (def.methods[n].name == proc_name) {
                return def.methods[n];
            }
        }
    }
    return null;
}

Node.prototype.make_mime_table = function (div_name, mimes) {
    var html = '<table class="table" >';
    html += '<thead><tr><th>extension</th><th>mime</th></tr></thead>';
    html += '<tbody>';
    for (var mimek in mimes) {
        html += '<tr><td>' + mimek + '</td><td>' + mimes[mimek] + '</td></tr>';
    }
    html += '</tbody>';
    html += '</table>';
    $('#' + div_name).html(html);
}



Node.prototype.setInfos = function (node)
{

    $('#node_name').html(node.user_agent);
    $('#node_version').html(node.version);
    $('#node_bheight').html(node.block_height);

    $('#node_port').html(node.p2p_addr.port);
    $('#node_addr').html(node.p2p_addr.addr);

    $('#lastnodeblock').html(node.last_block);
    /* $('#lastnodeblock').attr('href', '/nodix.site/block/' + node.last_block); */
    $('#lastnodeblock').attr('data-target', '#blockmodal');
    $('#lastnodeblock').attr('data-toggle', 'modal');

    $('#lastpowblock').html(node.lastPOWBlk);
    /* $$('#lastpowblock').attr('href', '/nodix.site/block/' + node.lastPOWBlk); */
    $('#lastpowblock').attr('data-target', '#blockmodal');
    $('#lastpowblock').attr('data-toggle', 'modal');

    $('#lastposblock').html(node.lastPOSBlk);
    /* $('#lastposblock').attr('href', '/nodix.site/block/' + node.lastPOSBlk); */
    $('#lastposblock').attr('data-target', '#blockmodal');
    $('#lastposblock').attr('data-toggle', 'modal');

    $('#currentpowdiff').html(node.current_pow_diff.toString(16));
    $('#currentposdiff').html(node.current_pos_diff.toString(16));

    $('#powreward').html(node.pow_reward);
    $('#posreward').html(node.pos_reward);
}

Node.prototype.updateBlockInfos = function () {
    var self = this;
    rpc_call('getinfo', [], function (data) {

        var infos = data.result;

        SelfNode.user_agent= infos.version;
        SelfNode.version = infos.protocolversion;
        SelfNode.block_height= infos.blocks;
        SelfNode.p2p_addr.port= infos.p2pport;
        SelfNode.p2p_addr.addr = infos.ip;

        SelfNode.current_pow_diff = infos.difficulty.ipow;
        SelfNode.current_pos_diff = infos.difficulty.ipos;

        SelfNode.pow_reward = infos.reward.mining;
        SelfNode.pos_reward = infos.reward.staking;

        $('#node_div').empty();
        self.setInfos(SelfNode);
        self.make_node_html('node_div', SelfNode);
    });

}

Node.prototype.seteventsrc = function (in_url, handler) {
    var self = this;

    this.evtSource = new EventSource(site_base_url + in_url);

    this.evtSource.addEventListener("newblock", handler, false);

}


var MyNode = null;