<script setup>
import { ref, onMounted } from 'vue'
import axios from 'axios';

const message = ref('Hello World!')

const items = ref([])

const fetchData = async () => {
  try {
    const response = await axios.get('https://jsonplaceholder.typicode.com/users')
    items.value = response.data
  } catch (error) {
    console.error(error)
  }
}

onMounted(() => {
  fetchData()
})
</script>

<template>
  <h1>{{ message }}</h1>
  <div id="app">
    <h1>Список данных</h1>
    <button @click="fetchData">Обновить данные</button>
    <ul>
      <li v-for="item in items" :key="item.id">{{ item.name }}</li>
    </ul>
  </div>
</template>
