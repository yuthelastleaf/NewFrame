<template>
    <div ref="graphDiv" style="width: 100%; height: 90%; border: 1px solid black;">
    </div>
    <div v-if="menuVisible" @contextmenu.prevent="handleRightClick" class="context-menu"
        :style="{ top: menuPosition.y + 'px', left: menuPosition.x + 'px' }">
        <ul>
            <li @click="menuAction('action1')">操作 1</li>
            <li @click="menuAction('action2')">操作 2</li>
            <li @click="menuAction('action3')">操作 3</li>
        </ul>
    </div>
    <el-button type="primary" style="height: 5%;" @click="handleClick">点击我</el-button>
</template>

<script setup>
import { onMounted, ref } from 'vue';
import { PegenGraph } from '../Graph/PegenGraph.js';

const graphDiv = ref(null);
const pegen_graph = new PegenGraph()

const menuVisible = ref(false);
const menuPosition = ref({ x: 0, y: 0 });

const handleClick = () => {
    console.log('按钮被点击了！');
    pegen_graph.addElement();
};

// const dropdownMenu = ref(null);
const menuAction = (action) => {
    console.log(`${action} 被点击了`);

    // 获取JointJS容器的位置
    const rect = graphDiv.value.getBoundingClientRect();

    pegen_graph.addElement(menuPosition.value.x - window.scrollX - rect.left,
     menuPosition.value.y - rect.top - window.scrollY);
    menuVisible.value = false;
};

// 右键点击事件处理函数
const handleRightClick = (event) => {
    // 阻止默认的右键菜单
    event.preventDefault();
};

onMounted(() => {

    pegen_graph.init(graphDiv);
    pegen_graph.pg_paper.on('cell:contextmenu', function (cellView, evt) {
        console.log("右键点击")
    });
    pegen_graph.addElement(270, 50);

    pegen_graph.pg_paper.on('blank:contextmenu', function (evt, x, y) {
        evt.preventDefault(); // 阻止默认的右键菜单

        // 获取JointJS容器的位置
        const rect = graphDiv.value.getBoundingClientRect();

        console.log('在空白处右键点击了', x, y);
        menuPosition.value = { x: rect.left + window.scrollX + x, y: rect.top + window.scrollY + y };
        menuVisible.value = true;
    });

});
</script>

<style>
.context-menu-wrapper {
    position: relative;
}

.target-area {
    width: 100%;
    height: 200px;
    background-color: #f0f0f0;
}

.context-menu {
    position: fixed;
    z-index: 1000;
    width: 150px;
    background: white;
    border: 1px solid #ddd;
    box-shadow: 2px 2px 4px #aaa;
}

.context-menu ul {
    list-style: none;
    margin: 0;
    padding: 5px 0;
}

.context-menu ul li {
    padding: 8px 12px;
    cursor: pointer;
}

.context-menu ul li:hover {
    background-color: #f5f5f5;
}
</style>