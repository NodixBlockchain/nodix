
function Composer(root_name,init_expression , opts)
{
    var self=this;
    var card,ins;
    var h2,label,div,button;

    this.parse_tree = null;
    this.parse_inputs = null;

    if (opts.hideT)
        this.showT = false;
    else
        this.showT = true;

    this.mainCard=document.createElement('div');
    this.mainCard.className = "card mb-3 bg-primary text-center composer-card";

    ins=document.createElement('div');
    ins.className="card-header";

        h2 = document.createElement('h2');
        h2.innerHTML = 'Abstract Synthax Tree';
        ins.appendChild(h2);

        this.graphRootHash = document.createElement('label');
        this.graphRootHash.id = 'graphRootHash';
        ins.appendChild(this.graphRootHash);
        

        if (init_expression != null) {
            label = document.createElement('label');
            label.setAttribute('for', 'expresion');
            label.innerHTML = 'expression';
            ins.appendChild(label);

            this.expression = document.createElement('input');
            this.expression.setAttribute('type', 'text');
            this.expression.id = "expression";
            this.expression.value = init_expression;
            this.expression.style.width = '90%';
            this.expression.style.fontSize = '10px';
            ins.appendChild(this.expression);

            button = document.createElement('button');
            button.innerHTML = 'parse expression';
            button.className = "btn btn-secondary";
            button.addEventListener('click', function (e) { self.build_tree(self.expression.value); })

            ins.appendChild(button);
        }
        else
            this.expression = null;

    this.mainCard.appendChild(ins);
    ins=document.createElement('div');
    ins.className="card-body";

        this.graphDiv=document.createElement('div');
        this.graphDiv.id = "graph";
        this.graphDiv.className = "graph";
        ins.appendChild(this.graphDiv);

    this.mainCard.appendChild(ins);
    ins=document.createElement('div');
    ins.className="card-footer";

    if (opts.createTx == true) {

        var div = document.createElement('div');
        div.className = 'container text-center';

        var row = document.createElement('div');
        row.className = 'row';

        var col = document.createElement('div');

            col.className = 'col';
            this.createTxBtn = document.createElement('button');
            this.createTxBtn.id = "createTxBtn";
            this.createTxBtn.className = "btn btn-secondary";
            this.createTxBtn.setAttribute("disabled", "disabled");
            this.createTxBtn.innerHTML = 'create txs';
            this.createTxBtn.addEventListener('click', function () { self.n_parsed_tx = 0; self.build_txs(self.parse_tree[0], tx_finished); })

            col.appendChild(this.createTxBtn);

        row.appendChild(col);

        div.appendChild(row);

        var row = document.createElement('div');
        row.className = 'row';

        var col = document.createElement('div');
            col.className = 'col-md-2';
        
            label = document.createElement('label');
            label.setAttribute('for', 'opRootHash');
            label.innerHTML = 'tree root node hash';
            col.appendChild(label);

        row.appendChild(col);

        var col = document.createElement('div');
            col.className = 'col-md-6';

            this.opRootHash = document.createElement('input');
            this.opRootHash.id = "opRootHash";
            this.opRootHash.setAttribute('type', 'text');
            /*this.opRootHash.setAttribute('size', '64');*/
            this.opRootHash.className = 'hash-input';

            this.opRootHash.setAttribute("disabled", "disabled");
            col.appendChild(this.opRootHash);
        row.appendChild(col);

   
        var col = document.createElement('div');
            col.className = 'col';

            this.opRootLink = document.createElement('a');
            this.opRootLink.id = "opRootLink";
            this.opRootLink.setAttribute('target', '_blank');
        col.appendChild(this.opRootLink);


        row.appendChild(col);
        div.appendChild(row);
        ins.appendChild(div);

        div = document.createElement('div');
        div.id = "tx_list";
       
        ins.appendChild(div);
    }
    else
        this.createTxBtn = null;

   
    this.mainCard.appendChild(ins);

    if (root_name != null)
        document.getElementById(root_name).appendChild(this.mainCard);
}


Composer.prototype.process_fn_arg = function (prev_done, obj) {
    var self = this;
    return rpc_call_promise('make_function_tx', [obj.name, obj.args[0]], true).done(function (data) { console.log("fn tx done " + obj.name); $('#tx_list').append(get_tx_html(data.result.transaction, self.n_parsed_tx)); self.n_parsed_tx++; obj.txid = data.result.transaction.txid; prev_done(); });
}

Composer.prototype.op_done = function (prev_done, obj) {
    var self = this;
    return rpc_call_promise('make_operation_tx', [obj.name, obj.args[0], obj.args[1]], true).done(function (data) { console.log("op tx done " + obj.name); $('#tx_list').append(get_tx_html(data.result.transaction, self.n_parsed_tx)); self.n_parsed_tx++; obj.txid = data.result.transaction.txid; prev_done(); });
}

Composer.prototype.process_op_arg2 = function (prev_done, obj) {
    var self = this;
    var my_arg2_done = function () { console.log("op done " + obj.name); return self.op_done(prev_done, obj); };
    var req;

    req = this.build_txs(obj.args[1], my_arg2_done);
    if (req != null)
        return req;

    return my_arg2_done();
}


Composer.prototype.build_txs = function (obj, prev_done) {
    var self = this;

    if (typeof obj == 'string') {
        console.log("var '" + obj + "'\n");
        return null;
    }

    if (typeof obj == 'number') {
        console.log("value '" + obj + "'\n");
        return null;
    }

    if (obj.type == 'op') {

        var req;
        var my_process_arg2 = function () { console.log("process arg2 " + obj.name); return self.process_op_arg2(prev_done, obj); };

        console.log("operation '" + obj.name + "'\n");

        req = self.build_txs(obj.args[0], my_process_arg2);
        if (req != null)
            return req;

        return my_process_arg2();
    }
    else if (obj.type == 'func') {


        var my_arg_done = function () { console.log("process fn " + obj.name); return self.process_fn_arg(prev_done, obj); };

        console.log("function '" + obj.name + "'\n");

        req = self.build_txs(obj.args[0], my_arg_done);
        if (req != null)
            return req;

        return my_arg_done();
    }

    return null;

}


function make_plotly_diag(data, prev, obj) {

    if (typeof obj == 'undefined')
        return 0;

    var curidx = data.node.label.length;

    if (obj.type == 'op') {

        obj.idx = curidx;

        data.node.label.push(obj.name);

        data.node.color.push("red");

        if (prev != null) {
            data.link.label.push(obj.txid);
            data.link.source.push(obj.idx);
            data.link.target.push(prev.idx);
            data.link.value.push(1);
        }



        make_plotly_diag(data, obj, obj.args[0]);
        make_plotly_diag(data, obj, obj.args[1]);

    }
    else if (obj.type == 'func') {

        obj.idx = curidx;

        data.node.label.push(obj.name);
        data.node.color.push("green");

        if (prev != null) {
            data.link.label.push(obj.txid);
            data.link.source.push(obj.idx);
            data.link.target.push(prev.idx);
            data.link.value.push(1);
        }



        make_plotly_diag(data, obj, obj.args[0]);

    }
    else if (typeof obj == 'string') {

        data.node.label.push('var ' + obj);
        data.node.color.push("blue");

        if (prev != null) {
            data.link.label.push('');
            data.link.source.push(curidx);
            data.link.target.push(prev.idx);
            data.link.value.push(1);
        }


    }
    else if (typeof obj.objKey == 'string') {

        data.node.label.push(obj.appName + ' obj key ' + obj.objKey);
        data.node.color.push("black");

        obj.idx = curidx;

        if (prev != null) {
            data.link.label.push('');
            data.link.source.push(obj.idx);
            data.link.target.push(prev.idx);
            data.link.value.push(1);
        }
    }
    else {

        data.node.label.push('value ' + obj.toString());
        data.node.color.push("black");

        if (prev != null) {
            data.link.label.push('');
            data.link.source.push(curidx);
            data.link.target.push(prev.idx);
            data.link.value.push(1);
        }
    }

}

function make_node(obj) {
    var html = '';

    if (obj.type == 'op') {
        html += '<div class="graph_op">';
        html += '<div class="graph_args">';
        html += make_node(obj.args[0]);
        html += make_node(obj.args[1]);
        html += '</div>';

        html += '<span style="vertical-align:middle">op ' + obj.name + "</span>";
        html += '</div>';
    }
    else if (obj.type == 'func') {

        html += '<div class="graph_args">';
        html += make_node(obj.args[0]);
        html += '</div>';
        html += '<div class="graph_func">';
        html += '<span style="vertical-align:middle">fn ' + obj.name + "</span>";
        html += '</div>';
    }
    else if (typeof obj == 'string') {
        html += '<div class="graph_var">var ' + obj + '</div>';
    }
    else if (typeof obj.objId == 'string') {
        html += '<div class="graph_var">' + obj.appName + ' obj ' + obj.objId + ' key ' + obj.objKey + '</div>';
    }

    else {
        html += '<div class="graph_val">value ' + obj + '</div>';
    }

    return html
}
Composer.prototype.read_input_obj = function (obj) {

    if (this.parse_inputs == null)
        return 0;

    var keys = Object.keys(this.parse_inputs);
    for (var n = 0; n < keys.length; n++) {
        if (keys[n] != 't') {
            if (typeof obj[keys[n]] != 'undefined')
                this.parse_inputs[keys[n]] = obj[keys[n]];
        }
    }
}


Composer.prototype.read_input_form = function () {

    if (this.parse_inputs == null)
        return 0;

    var keys = Object.keys(this.parse_inputs);

    for (var n = 0; n < keys.length; n++) {
        if (keys[n] != 't') {
            var var_val = parseFloat($('#var_' + keys[n]).val());
            this.parse_inputs[keys[n]] = var_val;
        }
    }

    this.parse_inputs.t = {value : 0};

    if (this.t_max_input != null)
        this.parse_inputs.t.end = parseInt(this.t_max_input.value);
    else
        this.parse_inputs.t.end = 1000;

    if (this.t_scaleFac_input != null)
        this.parse_inputs.t.scaleFac = parseInt(this.t_scaleFac_input.value);
    else
        this.parse_inputs.t.scaleFac = 44100;
}


Composer.prototype.create_inputs = function ()
{
    //var html = '<div class="container" >';

    if (this.parse_inputs == null)
        return;

    var self = this;
    var keys = Object.keys(this.parse_inputs);
    var root = document.createElement('div');
    root.className = 'container';

    var form = document.createElement('form');
    form.id='input_form';

    for (var n = 0; n < keys.length; n++) {

        if ((keys[n] != 't') && (keys[n] != 'scaleFac')) 
        {
            var var_id = 'var_' + keys[n];

            var row = document.createElement('div');
            row.className = 'row';

            var col = document.createElement('div');
                col.className = 'col-md-2';
                var label = document.createElement('label');
                label.setAttribute('for', var_id);
                label.innerHTML =  keys[n];
                col.appendChild(label);

            row.appendChild(col);

            var col = document.createElement('div');
                col.className = 'col-md-1';
                col.innerHTML =  '=';
            row.appendChild(col);

            var col = document.createElement('div');
                col.className = 'col-md-4';
                var input = document.createElement('input');
                input.setAttribute('type', 'text');
                input.id = var_id;

                if(this.parse_inputs[keys[n]])
                    input.value = this.parse_inputs[keys[n]];
                else
                    input.value = 1;

                col.appendChild(input);
            
            row.appendChild(col);
           
            form.appendChild(row);
        }
    }
  
    if (this.showT)
    {
        var row = document.createElement('div');
        row.className = 'row';

            var col = document.createElement('div');
                col.className = 'col-md-2';
                var label = document.createElement('label');
                label.setAttribute('for', 'var_t_max');
                label.innerHTML =  't';
                col.appendChild(label);
            row.appendChild(col);

            var col = document.createElement('div');
                col.className = 'col-md-1';
                col.innerHTML =  'ms';
            row.appendChild(col);

            var col = document.createElement('div');

                col.className = 'col-md-4';

                this.t_max_input = document.createElement('input');
                this.t_max_input.setAttribute('type', 'text');
                this.t_max_input.id = 'var_t_max';

                if (this.parse_inputs.t.end > 0)
                    this.t_max_input.value = this.parse_inputs.t.end;
                else
                    this.t_max_input.value = 1000;

                col.appendChild(this.t_max_input);

                row.appendChild(col);

       form.appendChild(row);

        var row = document.createElement('div');
        row.className = 'row';

            var col = document.createElement('div');
                col.className = 'col-md-2';
                var label = document.createElement('label');
                label.setAttribute('for', 'var_t_scaleFac');
                label.innerHTML = 'sample per second';
                col.appendChild(label);
            row.appendChild(col);

            var col = document.createElement('div');
                col.className = 'col-md-1';
                col.innerHTML =  'unit/sec';
            row.appendChild(col);

            var col = document.createElement('div');

                col.className = 'col-md-4';

                    this.t_scaleFac_input = document.createElement('input');
                    this.t_scaleFac_input.setAttribute('type', 'text');
                    this.t_scaleFac_input.id = 'var_t_scaleFac';

                    if (this.parse_inputs.t.scaleFac > 0)
                        this.t_scaleFac_input.value = this.parse_inputs.t.scaleFac;
                    else
                        this.t_scaleFac_input.value = 48000;

                    col.appendChild(this.t_scaleFac_input);

                    row.appendChild(col);

        form.appendChild(row);

        this.executeBtn = document.createElement('button');
        this.executeBtn.id = "executeBtn";
        this.executeBtn.type = "button";
        this.executeBtn.className = "btn btn-secondary";
        this.executeBtn.innerHTML = 'execute tree';
        this.executeBtn.addEventListener('click', function (e) {

            $('#playGenBtn').prop('disabled', 'disabled');
            $('#genPlot').html('<img width=\'80%\' src=\'/assets/img/loading.gif\'/>');

            self.read_input_form();

            self.execute_tree().onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    var out = getOutputBuffer(this.responseText);
                    data_finished(out.values, out.smin, out.smax);
                }
            }
            e.preventDefault();
            return false;
        });
        form.appendChild(this.executeBtn);
    }
    else
    {
        this.t_scaleFac_input = null;
        this.t_max_input = null;
        this.executeBtn = null;
    }
   

    

   
    root.appendChild(form);

    return root;
}

Composer.prototype.graphDone = function () {

}

Composer.prototype.load_tree = function (hash) {

    var self = this;

    this.graphDiv.innerHTML = '<img width="80%" src="/assets/img/loading.gif" />'

    rpc_call('load_op_graph', [hash], function (data) {

        if (!data.error) {
            self.parse_tree = data.result.tree;
            self.parse_inputs = data.result.inputs;

            self.graphDiv.innerHTML = '';

            if (self.createTxBtn)
                self.createTxBtn.setAttribute('disabled', 'disabled');

         

            self.graphDone();
         
        }
        else {
            self.graphDiv.innerHTML = 'error';
        }
    }, function (e) { this.graphDiv.innerHTML = ' error'; });
}



Composer.prototype.build_tree = function (expression) {

    var self = this;

    this.graphDiv.innerHTML = '<img width="80%" src="/assets/img/loading.gif" />';


     rpc_call('eval_combinator', [expression], function (data) {

         if (!data.error)
         {
             self.parse_tree = data.result.tree;
             self.parse_inputs = data.result.inputs;

             self.graphDiv.innerHTML = '';

             if (self.createTxBtn)
                 self.createTxBtn.removeAttribute('disabled');
           

             self.graphDone();

         }
         else {
             self.graphDiv.innerHTML = 'error';
         }
    }, function (e) { this.graphDiv.innerHTML =' error'; });
}




function getOutputBuffer(resp) {
    var buffer = new ArrayBuffer(resp.length);
    var dataview = new DataView(buffer);
    var byteArray = new Uint8Array(buffer);

    for (var i = 0; i < resp.length; i++) {
        byteArray[i] = resp.charCodeAt(i) & 0xff;
    }

    var smin = null;
    var smax = null;
    var flen = Math.ceil(resp.length / 4);

    var values = new Float32Array(flen);

    for (var i = 0; i < flen; i++) {
        values[i] = dataview.getFloat32(i * 4, true);

        if (isNaN(values[i]))
            values[i] = 0.0;
        else {
            if (smin == null)
                smin = values[i];
            else
                smin = Math.min(smin, values[i]);

            if (smax == null)
                smax = values[i];
            else
                smax = Math.max(smax, values[i]);
        }
    }

    return {values:values, smin:smin,smax:smax};
}

Composer.prototype.execute_tree = function (ready_func) {

    
    var req = new XMLHttpRequest();
    req.open('POST', api_base_url + rpc_base, true);
    req.overrideMimeType('text\/plain; charset=x-user-defined');
    req.send(JSON.stringify({ jsonrpc: '2.0', method: 'execute_tree', params: [this.parse_inputs, this.parse_tree], id: 1 }));
    return req;
}

var MyComposer = null;